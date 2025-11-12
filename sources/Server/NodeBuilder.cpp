#include "NodeBuilder.hpp"
#include "CheckStatus.hpp"
#include "StringConverter.hpp"
#include "VariantConverter.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <Variant_Visitor/Visitor.hpp>
#include <open62541/nodeids.h>
#include <open62541/statuscodes.h>

#include <string>
#include <vector>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace open62541;

constexpr UA_UInt16 SERVER_NAMESPACE = 1;

struct NodeMetaInfo {
  NodeMetaInfo(const MetaInfoPtr& element, optional<UA_NodeId> parent_node_id) {
    id = UA_NODEID_STRING_ALLOC(SERVER_NAMESPACE, element->id().c_str());

    bool is_root = !parent_node_id.has_value();
    // if no parent has been provided, set to root objects folder
    if (is_root) {
      parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    } else {
      UA_NodeId_copy(&(parent_node_id.value()), &parent);
    }

    reference_type = UA_NODEID_NUMERIC(
        0, (is_root ? UA_NS0ID_ORGANIZES : UA_NS0ID_HASCOMPONENT));

    name = UA_QUALIFIEDNAME_ALLOC(SERVER_NAMESPACE, element->name().c_str());
  }

  ~NodeMetaInfo() {
    UA_NodeId_clear(&id);
    UA_NodeId_clear(&parent);
    UA_NodeId_clear(&reference_type);
    UA_QualifiedName_clear(&name);
  }

  UA_NodeId id;
  UA_NodeId parent;
  UA_NodeId reference_type;
  UA_QualifiedName name;
};

NodeBuilder::NodeBuilder(const CallbackRepoPtr& repo,
#ifdef ENABLE_UA_HISTORIZING
    const HistorizerPtr& historizer,
#endif // ENABLE_UA_HISTORIZING
    UA_Server* server)
    : logger_(LoggerManager::registerLogger("Open62541::NodeBuilder")),
      repo_(repo),
#ifdef ENABLE_UA_HISTORIZING
      historizer_(historizer),
#endif // ENABLE_UA_HISTORIZING
      server_(server) {
}

#ifdef ENABLE_UA_HISTORIZING
void NodeBuilder::historize(UA_NodeId node_id, const UA_DataType* type) {
  if (!historizer_) {
    logger_->info("Historizer is not set");
    return;
  }
  try {
    auto status = historizer_->registerNodeId(server_, node_id, type);
    checkStatusCode(
        "While registering writable node historization callback", status);
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to historize node {} due to exception {}",
        toString(&node_id), ex.what());
  }
}
#endif // ENABLE_UA_HISTORIZING

UA_NodeId NodeBuilder::addObjectNode(
    const MetaInfoPtr& element, optional<UA_NodeId> parent_node_id) {
  logger_->info(
      "Adding a new node: {}, with id: {}", element->name(), element->id());

  auto type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);

  auto node = NodeMetaInfo(element, parent_node_id);
  auto object_attributes = UA_ObjectAttributes_default;
  object_attributes.displayName =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->name().c_str());
  object_attributes.description =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->description().c_str());

  UA_NodeId result;
  auto status = UA_Server_addObjectNode(server_, node.id, node.parent,
      node.reference_type, node.name, type_definition, object_attributes,
      nullptr, &result);

  checkStatusCode(
      "While creating Object Node for " + element->id() + " " + element->name(),
      status);

  UA_ObjectAttributes_clear(&object_attributes);

  return result;
}

UA_StatusCode NodeBuilder::addDeviceNode(const DevicePtr& device) {
  try {
    auto parent_id = addObjectNode(device);
    device->visit([this, parent_id](const ElementPtr& element) {
      try {
        auto status = addElementNode(element, parent_id);
        checkStatusCode("While adding DeviceElement " + element->id() + " " +
                element->name() + " node",
            status);

      } catch (const StatusCodeNotGood& ex) {
        logger_->error("Failed to create an Element Node {}:{} for Device Node "
                       "{}. Status: {}",
            element->id(), element->name(), toString(&parent_id), ex.what());
      }
    });
    UA_NodeId_clear(&parent_id);
    return UA_STATUSCODE_GOOD;
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to create a Node for Device {}:{}. Status: {}",
        device->id(), device->name(), ex.what());
    return UA_STATUSCODE_BADINTERNALERROR;
  }
}

UA_StatusCode NodeBuilder::removeDataSources(const UA_NodeId* node_id) {
  UA_StatusCode result = UA_STATUSCODE_BAD;
  UA_BrowseDescription browse_description{// clang-format off
      .nodeId = *node_id,
      .browseDirection = UA_BROWSEDIRECTION_FORWARD,
      .referenceTypeId = UA_NODEID_NULL,
      .includeSubtypes = UA_TRUE,
      .nodeClassMask = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE | UA_NODECLASS_METHOD,
      .resultMask = UA_BROWSERESULTMASK_NODECLASS
  }; // clang-format on

  logger_->trace(
      "Browsing Node {} for subnodes with callbacks", toString(node_id));
  auto browse_result = UA_Server_browse(server_, 0, &browse_description);

  for (size_t i = 0; i < browse_result.referencesSize; ++i) {
    auto node_reference = browse_result.references[i];
    if (node_reference.nodeClass != UA_NODECLASS_OBJECT) {
      repo_->remove(&node_reference.nodeId.nodeId);
      result = UA_STATUSCODE_GOOD;
    } else {
      result = removeDataSources(&node_reference.nodeId.nodeId);
    }
  }

  UA_BrowseResult_clear(&browse_result);
  return result;
}

UA_StatusCode NodeBuilder::deleteDeviceNode(const string& device_id) {
  auto device_node_id =
      UA_NODEID_STRING_ALLOC(SERVER_NAMESPACE, device_id.c_str());
  logger_->trace("Removing Node {}", toString(&device_node_id));

  auto result = removeDataSources(&device_node_id);

  if (UA_StatusCode_isBad(result)) {
    logger_->error(
        "Could not remove data sources for {} device node", device_id);
  } else {
    logger_->trace("Removed linked data sources for device node {}", device_id);
  }

  result = UA_Server_deleteNode(server_, device_node_id, true);
  if (UA_StatusCode_isBad(result)) {
    logger_->error("Could not delete {} device node: {}", device_id,
        string(UA_StatusCode_name(result)));
  } else {
    logger_->trace("Device node {} deleted", device_id);
  }

  UA_NodeId_clear(&device_node_id);
  return result;
}

UA_StatusCode NodeBuilder::addElementNode(
    const ElementPtr& element, const UA_NodeId& parent_id) {
  logger_->info(
      "Adding element {} to node {}", element->name(), toString(&parent_id));

  return Variant_Visitor::match(
      element->function(),
      [&](const GroupPtr& group) {
        return addGroupNode(element, group, parent_id);
      },
      [&](const ObservablePtr& observable) {
        return addObservableNode(element, observable, parent_id);
      },
      [&](const ReadablePtr& readable) {
        return addReadableNode(element, readable, parent_id);
      },
      [&](const WritablePtr& writable) {
        return addWritableNode(element, writable, parent_id);
      },
      [&](const CallablePtr& callable) {
        return addCallableNode(element, callable, parent_id);
      });
}

UA_StatusCode NodeBuilder::addGroupNode(const MetaInfoPtr& meta_info,
    const GroupPtr& group, const UA_NodeId& parent_id) {
  logger_->trace("Adding Group Node for element {}:{}", meta_info->id(),
      meta_info->name());
  try {
    auto node_id = addObjectNode(meta_info, parent_id);
    group->visit([this, node_id](const ElementPtr& element) {
      try {
        auto status = addElementNode(element, node_id);
        checkStatusCode("While adding DeviceElement " + element->id() + " " +
                element->name() + " node",
            status);
      } catch (const StatusCodeNotGood& ex) {
        logger_->error(
            "Failed to create element {}:{} for parent node {}. Status: {}",
            element->id(), element->name(), toString(&node_id), ex.what());
      }
    });
    UA_NodeId_clear(&node_id);
    return UA_STATUSCODE_GOOD;
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to create a Node for Group {}:{}. Status: {}",
        meta_info->id(), meta_info->name(), ex.what());
    return UA_STATUSCODE_BADINTERNALERROR;
  }
}

UA_VariableAttributes setValueAttributes(
    const MetaInfoPtr& meta_info, DataType type) {
  UA_VariableAttributes value_attributes = UA_VariableAttributes_default;
  value_attributes.description =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", meta_info->description().c_str());
  value_attributes.displayName =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", meta_info->name().c_str());
  value_attributes.dataType = toNodeId(type);

  // open62541 seems to require some default value set, even if the node is
  // write-only
  auto default_value = setVariant(type);
  // this will always have a value, since toNodeId() throws if type is not
  // supported
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  value_attributes.value = toUAVariant(default_value.value());

  return value_attributes;
}

UA_VariableAttributes setValueAttributes(
    const MetaInfoPtr& meta_info, const DataVariant& value, DataType type) {
  auto value_attributes = setValueAttributes(meta_info, type);
  value_attributes.value = toUAVariant(value);
  return value_attributes;
}

UA_StatusCode NodeBuilder::addReadableNode(const MetaInfoPtr& meta_info,
    const ReadablePtr& readable, const UA_NodeId& parent_id) {
  logger_->trace("Adding Readable Node for element {}:{}", meta_info->id(),
      meta_info->name());
  auto node = NodeMetaInfo(meta_info, parent_id);
  try {
    auto value_attributes =
        setValueAttributes(meta_info, readable->read(), readable->dataType());
    value_attributes.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef ENABLE_UA_HISTORIZING
    value_attributes.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    value_attributes.historizing = true;
    historize(node.id, value_attributes.value.type);
#endif // ENABLE_UA_HISTORIZING
    auto status = repo_->add(node.id, readable);
    checkStatusCode("While setting readable metric callbacks", status);

    UA_DataSource data_source;
    data_source.read = &readNodeValue;
    data_source.write = nullptr;

    UA_NodeId type_definition =
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    status = UA_Server_addDataSourceVariableNode(server_, node.id, node.parent,
        node.reference_type, node.name, type_definition, value_attributes,
        data_source, repo_.get(), nullptr);
    checkStatusCode("While adding readable variable node to server", status);
    UA_NodeId_clear(&type_definition);
    UA_VariableAttributes_clear(&value_attributes);
    return status;
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Readable element {}:{}. Status: {}",
        meta_info->id(), meta_info->name(), ex.what());
    repo_->remove(&(node.id));
    return UA_STATUSCODE_BADINTERNALERROR;
  }
}

UA_StatusCode NodeBuilder::addObservableNode(
    const Information_Model::MetaInfoPtr& meta_info,
    const Information_Model::ObservablePtr& observable,
    const UA_NodeId& parent_id) {
  logger_->trace("Adding Observable Node for element {}:{}", meta_info->id(),
      meta_info->name());
  auto node = NodeMetaInfo(meta_info, parent_id);
  try {
    auto status = repo_->add(node.id, observable);
    checkStatusCode("While setting readable metric callbacks", status);

    auto value_attributes = setValueAttributes(
        meta_info, observable->read(), observable->dataType());
    value_attributes.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef ENABLE_UA_HISTORIZING
    value_attributes.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    value_attributes.historizing = true;
    historize(node.id, value_attributes.value.type);
#endif // ENABLE_UA_HISTORIZING
    UA_DataSource data_source;
    data_source.read = &readNodeValue;
    data_source.write = nullptr;

    UA_NodeId type_definition =
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    status = UA_Server_addDataSourceVariableNode(server_, node.id, node.parent,
        node.reference_type, node.name, type_definition, value_attributes,
        data_source, repo_.get(), nullptr);
    checkStatusCode("While adding readable variable node to server", status);

    UA_NodeId_clear(&type_definition);
    UA_VariableAttributes_clear(&value_attributes);
    return status;
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Observable element {}:{}. Status: {}",
        meta_info->id(), meta_info->name(), ex.what());
    repo_->remove(&(node.id));
    return UA_STATUSCODE_BADINTERNALERROR;
  }
}

UA_StatusCode NodeBuilder::addWritableNode(const MetaInfoPtr& meta_info,
    const WritablePtr& writable, const UA_NodeId& parent_id) {
  logger_->trace("Adding Writable Node for element {}:{}", meta_info->id(),
      meta_info->name());
  auto node = NodeMetaInfo(meta_info, parent_id);
  try {
    auto status = repo_->add(node.id, writable);
    checkStatusCode("While setting writable metric callbacks", status);

    auto value_attributes = setValueAttributes(meta_info, writable->dataType());
    // trying to create a write only node, results in an internal error
    value_attributes.accessLevel =
        UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    UA_DataSource data_source;
    if (!writable->isWriteOnly()) {
      value_attributes.value = toUAVariant(writable->read());
#ifdef ENABLE_UA_HISTORIZING
      value_attributes.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
      value_attributes.historizing = true;
      historize(node.id, value_attributes.value.type);
#endif // ENABLE_UA_HISTORIZING
    }
    data_source.read = &readNodeValue;
    data_source.write = &writeNodeValue;

    UA_NodeId type_definition =
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    status = UA_Server_addDataSourceVariableNode(server_, node.id, node.parent,
        node.reference_type, node.name, type_definition, value_attributes,
        data_source, repo_.get(), nullptr);
    checkStatusCode("While adding writable variable node to server", status);
    UA_NodeId_clear(&type_definition);
    UA_VariableAttributes_clear(&value_attributes);
    return status;
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Writable element {}:{}. Status: {}",
        meta_info->id(), meta_info->name(), ex.what());
    repo_->remove(&(node.id));
    return UA_STATUSCODE_BADINTERNALERROR;
  }
}

UA_Argument* makeInputArgs(ParameterTypes params) {
  UA_Argument* result = nullptr;
  if (!params.empty()) {
    result = (UA_Argument*)malloc(params.size() * sizeof(UA_Argument));
    for (size_t i = 0; i < params.size(); ++i) {
      UA_Argument_init(&result[i]);
      auto parameter = params.at(i);
      result[i].name = makeUAString(toString(parameter.type));
      result[i].dataType = toNodeId(parameter.type);
      result[i].valueRank = UA_VALUERANK_SCALAR; // each input type is scalar
      auto arg_desc =
          (parameter.mandatory ? "Mandatory " : "") + toString(parameter.type);
      result[i].description = UA_LOCALIZEDTEXT_ALLOC("EN_US", arg_desc.c_str());
    }
  }
  return result;
}

UA_Argument* makeOutputType(DataType type) {
  UA_Argument* result = nullptr;
  if (type != DataType::None) {
    result = UA_Argument_new();
    result->name = makeUAString(toString(type));
    result->dataType = toNodeId(type);
    result->valueRank = UA_VALUERANK_SCALAR; // all returns are scalar
    result->description =
        UA_LOCALIZEDTEXT_ALLOC("EN_US", toString(type).c_str());
  }
  return result;
}

UA_StatusCode NodeBuilder::addCallableNode(const MetaInfoPtr& meta_info,
    const CallablePtr& callable, const UA_NodeId& parent_id) {
  logger_->trace("Adding Callable Node for element {}:{}", meta_info->id(),
      meta_info->name());
  auto status = UA_STATUSCODE_BADINTERNALERROR;
  UA_Argument* input_args = makeInputArgs(callable->parameterTypes());
  UA_Argument* output = makeOutputType(callable->resultType());
  auto node = NodeMetaInfo(meta_info, parent_id);
  try {
    status = repo_->add(node.id, callable);
    checkStatusCode("While setting executable callbacks", status);

    auto method_attributes = UA_MethodAttributes_default;
    method_attributes.description =
        UA_LOCALIZEDTEXT_ALLOC("en-US", meta_info->description().c_str());
    method_attributes.displayName =
        UA_LOCALIZEDTEXT_ALLOC("en-US", meta_info->name().c_str());
    method_attributes.executable = true;
    method_attributes.userExecutable = true;

    status = UA_Server_addMethodNode(server_, node.id, node.parent,
        node.reference_type, node.name, method_attributes, &callNodeMethod,
        callable->parameterTypes().size(), input_args,
        output == nullptr ? 0 : 1, output, repo_.get(), nullptr);
    checkStatusCode("While adding method node to server", status);
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Callable element {}:{}. Status: {}",
        meta_info->id(), meta_info->name(), ex.what());
    repo_->remove(&(node.id));
  }

  // clean local pointer instances to avoid memory leaks
  if (input_args != nullptr) {
    // input args are copied into method node or are not used in case of
    // failure
    for (size_t i = 0; i < callable->parameterTypes().size(); ++i) {
      UA_Argument_delete(&input_args[i]);
    }
  }
  if (output != nullptr) {
    // output is copied into method node or not used in case of failure
    UA_Argument_delete(output);
  }
  return status;
}