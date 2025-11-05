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

NodeBuilder::NodeBuilder(const shared_ptr<Open62541Server>& server)
    : logger_(LoggerManager::registerTypedLogger(this)), server_(server) {
  NodeCallbackHandler::initialise(server->getServerLogger());
}

NodeBuilder::~NodeBuilder() { cleanup(); }

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void NodeBuilder::cleanup() { NodeCallbackHandler::destroy(); }

pair<UA_StatusCode, UA_NodeId> NodeBuilder::addObjectNode(
    const MetaInfoPtr& element, optional<UA_NodeId> parent_node_id) {
  bool is_root = !parent_node_id.has_value();

  logger_->info(
      "Adding a new node: {}, with id: {}", element->name(), element->id());

  auto node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), element->id().c_str());
  logger_->trace("Assigning {} NodeId to element: {}, with id: {}",
      toString(&node_id), element->name(), element->id());

  auto reference_type_id = UA_NODEID_NUMERIC(
      0, (is_root ? UA_NS0ID_ORGANIZES : UA_NS0ID_HASCOMPONENT));

  // if no parent has been provided, set to root objects folder
  if (is_root) {
    parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  }

  auto browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), element->name().c_str());
  logger_->trace("Assigning browse name: {} to element: {}, with id: {}",
      toString(&browse_name), element->name(), element->id());

  auto type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);

  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;
  node_attr.description =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->description().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->name().c_str());

  auto status = UA_Server_addObjectNode(server_->getServer(), node_id,
      parent_node_id.value(), reference_type_id, browse_name, type_definition,
      node_attr, nullptr, nullptr);

  UA_QualifiedName_clear(&browse_name);
  UA_ObjectAttributes_clear(&node_attr);

  return make_pair(status, node_id);
}

UA_StatusCode NodeBuilder::addDeviceNode(const DevicePtr& device) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  auto result = addObjectNode(device);
  status = result.first;

  try {
    checkStatusCode("While creating Object Node for " + device->id() + " " +
            device->name() + " device",
        status);
    auto elements = device->group(); // @todo refactor to visitor
    for (const auto& element : elements->asVector()) {
      status = addElementNode(element, result.second);
      checkStatusCode("While adding DeviceElement " + element->id() + " " +
              element->name() + " node",
          status);
    }
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to create a Node for Device: {}. Status: {}",
        device->name(), ex.what());
  }
  UA_NodeId_clear(&result.second);
  return status;
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
  auto browse_result =
      UA_Server_browse(server_->getServer(), 0, &browse_description);

  for (size_t i = 0; i < browse_result.referencesSize; ++i) {
    auto node_reference = browse_result.references[i];
    if (node_reference.nodeClass != UA_NODECLASS_OBJECT) {
      result = NodeCallbackHandler::removeNodeCallbacks(
          &node_reference.nodeId.nodeId);
    } else {
      result = removeDataSources(&node_reference.nodeId.nodeId);
    }
  }

  UA_BrowseResult_clear(&browse_result);
  return result;
}

UA_StatusCode NodeBuilder::deleteDeviceNode(const string& device_id) {
  auto device_node_id =
      UA_NODEID_STRING_ALLOC(server_->getServerNamespace(), device_id.c_str());
  logger_->trace("Removing Node {}", toString(&device_node_id));

  auto result = removeDataSources(&device_node_id);

  if (UA_StatusCode_isBad(result)) {
    logger_->error(
        "Could not remove data sources for {} device node", device_id);
  } else {
    logger_->trace("Removed linked data sources for device node {}", device_id);
  }

  result = UA_Server_deleteNode(server_->getServer(), device_node_id, true);
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
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->info(
      "Adding element {} to node {}", element->name(), toString(&parent_id));

  Variant_Visitor::match(
      element->function(),
      [&](const GroupPtr& group) {
        status = addGroupNode(element, group, parent_id);
      },
      [&](const ObservablePtr& metric) {
        status = addObservableNode(element, metric, parent_id);
      },
      [&](const ReadablePtr& metric) {
        status = addReadableNode(element, metric, parent_id);
      },
      [&](const WritablePtr& metric) {
        status = addWritableNode(element, metric, parent_id);
      },
      [&](const CallablePtr& callable) {
        status = addCallableNode(element, callable, parent_id);
      });

  if (status != UA_STATUSCODE_GOOD) {
    logger_->error("Failed to create a Node for Device Element: {}. Status: {}",
        element->name(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::addGroupNode(const MetaInfoPtr& meta_info,
    const GroupPtr& group, const UA_NodeId& parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  auto result = addObjectNode(meta_info, parent_id);
  status = result.first;
  try {
    checkStatusCode("Parent's " + toString(&parent_id) + " group element " +
            meta_info->name() + " with id " + meta_info->id() + " is empty.",
        status);
    auto elements = group->asVector();

    logger_->info("Group element {}:{} contains {} subelements.",
        meta_info->name(), toString(&result.second), elements.size());
    for (const auto& element : elements) {
      status = addElementNode(element, result.second);
    }
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Device Element Group: {}. Status: {}",
        meta_info->name(), ex.what());
  }
  UA_NodeId_clear(&result.second);

  return status;
}

template <class MetricType>
UA_StatusCode NodeBuilder::setValue(UA_VariableAttributes& value_attribute,
    const MetaInfoPtr& meta_info, const shared_ptr<MetricType>& metric,
    const string& metric_type_description) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  try {
    auto variant = metric->read();
    value_attribute.value = toUAVariant(variant);
    value_attribute.description =
        UA_LOCALIZEDTEXT_ALLOC("EN_US", meta_info->description().c_str());
    value_attribute.displayName =
        UA_LOCALIZEDTEXT_ALLOC("EN_US", meta_info->name().c_str());
    value_attribute.dataType = toNodeId(metric->dataType());
    status = UA_STATUSCODE_GOOD;
  } catch (const exception& ex) {
    logger_->error("An exception occurred while trying to set " +
            metric_type_description + " value! Exception: {}",
        ex.what());
  }

  return status;
}

UA_StatusCode NodeBuilder::addReadableNode(const MetaInfoPtr& meta_info,
    const ReadablePtr& metric, const UA_NodeId& parent_id) {
  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->id().c_str());
  logger_->trace("Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->name(), meta_info->id());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->name().c_str());
  logger_->trace("Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->name(), meta_info->id());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  auto status = setValue(node_attr, meta_info, metric, "readable metric");
  try {
    checkStatusCode("While setting default readable metric value", status);
    logger_->trace("Assigning {} read callback for {} node",
        toString(metric->dataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef ENABLE_UA_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif // ENABLE_UA_HISTORIZING
    status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
        make_shared<CallbackWrapper>(metric->dataType(),
            (CallbackWrapper::ReadCallback)bind(&Readable::read, metric)));
    checkStatusCode("While setting readable metric callbacks", status);

    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;
    data_source.write = nullptr;

    auto* server_ptr = server_->getServer();
    status = UA_Server_addDataSourceVariableNode(server_ptr, metrid_node_id,
        parent_id, reference_type_id, metric_browse_name, type_definition,
        node_attr, data_source, nullptr, nullptr);
    checkStatusCode("While adding readable variable node to server", status);
#ifdef ENABLE_UA_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif // ENABLE_UA_HISTORIZING
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Readable Metric: {}. Status: {}",
        meta_info->name(), ex.what());
  }
  UA_NodeId_clear(&metrid_node_id);
  UA_QualifiedName_clear(&metric_browse_name);
  UA_VariableAttributes_clear(&node_attr);
  return status;
}

UA_StatusCode NodeBuilder::addObservableNode(
    const Information_Model::MetaInfoPtr& meta_info,
    const Information_Model::ObservablePtr& metric,
    const UA_NodeId& parent_id) {
  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->id().c_str());
  logger_->trace("Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->name(), meta_info->id());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->name().c_str());
  logger_->trace("Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->name(), meta_info->id());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  auto status = setValue(node_attr, meta_info, metric, "readable metric");
  try {
    checkStatusCode("While setting default readable metric value", status);
    logger_->trace("Assigning {} read callback for {} node",
        toString(metric->dataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef ENABLE_UA_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif // ENABLE_UA_HISTORIZING
    status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
        make_shared<CallbackWrapper>(metric->dataType(),
            (CallbackWrapper::ReadCallback)bind(&Observable::read, metric)));
    checkStatusCode("While setting readable metric callbacks", status);

    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;
    data_source.write = nullptr;

    auto* server_ptr = server_->getServer();
    status = UA_Server_addDataSourceVariableNode(server_ptr, metrid_node_id,
        parent_id, reference_type_id, metric_browse_name, type_definition,
        node_attr, data_source, nullptr, nullptr);
    checkStatusCode("While adding readable variable node to server", status);
#ifdef ENABLE_UA_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif // ENABLE_UA_HISTORIZING
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Readable Metric: {}. Status: {}",
        meta_info->name(), ex.what());
  }
  UA_NodeId_clear(&metrid_node_id);
  UA_QualifiedName_clear(&metric_browse_name);
  UA_VariableAttributes_clear(&node_attr);
  return status;
}

UA_StatusCode NodeBuilder::addWritableNode(const MetaInfoPtr& meta_info,
    const WritablePtr& metric, const UA_NodeId& parent_id) {
  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->id().c_str());
  logger_->trace("Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->name(), meta_info->id());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->name().c_str());
  logger_->trace("Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->name(), meta_info->id());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  auto status = setValue(node_attr, meta_info, metric, "writable metric");
  try {
    checkStatusCode("While setting default writable metric value", status);
    logger_->trace("Assigning {} read and write callbacks for {} node",
        toString(metric->dataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
#ifdef ENABLE_UA_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif // ENABLE_UA_HISTORIZING
    UA_DataSource data_source;
    if (!metric->isWriteOnly()) {
      status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
          make_shared<CallbackWrapper>(metric->dataType(),
              (CallbackWrapper::ReadCallback)bind(&Writable::read, metric),
              (CallbackWrapper::WriteCallback)bind(
                  &Writable::write, metric, placeholders::_1)));
      data_source.read = &NodeCallbackHandler::readNodeValue;
    } else {
      status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
          make_shared<CallbackWrapper>(metric->dataType(),
              (CallbackWrapper::WriteCallback)bind(
                  &Writable::write, metric, placeholders::_1)));
      data_source.read = nullptr;
    }
    checkStatusCode("While setting writable metric callbacks", status);
    data_source.write = &NodeCallbackHandler::writeNodeValue;

    status = UA_Server_addDataSourceVariableNode(server_->getServer(),
        metrid_node_id, parent_id, reference_type_id, metric_browse_name,
        type_definition, node_attr, data_source, nullptr, nullptr);
    checkStatusCode("While adding writable variable node to server", status);
#ifdef ENABLE_UA_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif // ENABLE_UA_HISTORIZING
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Writable Metric: {}. Status: {}",
        meta_info->name(), ex.what());
  }
  UA_NodeId_clear(&metrid_node_id);
  UA_QualifiedName_clear(&metric_browse_name);
  UA_VariableAttributes_clear(&node_attr);
  return status;
}

UA_StatusCode NodeBuilder::addCallableNode(const MetaInfoPtr& meta_info,
    const CallablePtr& callable, const UA_NodeId& parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  auto method_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->id().c_str());
  if (callable->resultType() != DataType::None &&
      callable->resultType() != DataType::Unknown) {
    CallbackWrapper::CallCallback call_cb =
        [callable, id = meta_info->id(), logger = server_->getServerLogger()](
            const Information_Model::Parameters& params) {
          try {
            return callable->call(params);
          } catch (const exception& ex) {
            UA_LOG_ERROR(logger, UA_LOGCATEGORY_SERVER,
                "An exception occurred while calling %s callable: %s",
                id.c_str(), ex.what());
            return Information_Model::DataVariant();
          }
        };
    status = NodeCallbackHandler::addNodeCallbacks(method_node_id,
        make_shared<CallbackWrapper>(
            callable->resultType(), callable->parameterTypes(), move(call_cb)));
  } else {
    CallbackWrapper::ExecuteCallback execute_cb =
        [callable, id = meta_info->id(), logger = server_->getServerLogger()](
            const Information_Model::Parameters& params) {
          try {
            callable->execute(params);
          } catch (const exception& ex) {
            UA_LOG_ERROR(logger, UA_LOGCATEGORY_SERVER,
                "An exception occurred while executing %s callable: %s",
                id.c_str(), ex.what());
          }
        };
    status = NodeCallbackHandler::addNodeCallbacks(method_node_id,
        make_shared<CallbackWrapper>(callable->resultType(),
            callable->parameterTypes(), move(execute_cb)));
  }
  UA_Argument* input_args = nullptr;
  UA_Argument* output = nullptr;
  try {
    checkStatusCode("While setting callable callbacks", status);
    auto arg_count = callable->parameterTypes().size();
    if (arg_count > 0) {
      input_args = (UA_Argument*)malloc(arg_count * sizeof(UA_Argument));
      for (size_t i = 0; i < callable->parameterTypes().size(); ++i) {
        UA_Argument_init(&input_args[i]);
        auto parameter = callable->parameterTypes().at(i);
        input_args[i].name = makeUAString(toString(parameter.type));
        input_args[i].dataType = toNodeId(parameter.type);
        input_args[i].valueRank =
            UA_VALUERANK_SCALAR; // each input type is scalar
        auto arg_desc = (parameter.mandatory ? "Mandatory " : "") +
            toString(parameter.type);
        input_args[i].description =
            UA_LOCALIZEDTEXT_ALLOC("EN_US", arg_desc.c_str());
      }
    }
    uint8_t output_count = 0;
    if (callable->resultType() != DataType::None) {
      output_count = 1;
      output = UA_Argument_new();
      output->name = makeUAString(toString(callable->resultType()));
      output->dataType = toNodeId(callable->resultType());
      output->valueRank = UA_VALUERANK_SCALAR; // all returns are scalar
      auto ouput_desc = toString(callable->resultType());
      output->description = UA_LOCALIZEDTEXT_ALLOC("EN_US", ouput_desc.c_str());
    }
    auto reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
    auto method_browse_name = UA_QUALIFIEDNAME_ALLOC(
        server_->getServerNamespace(), meta_info->name().c_str());

    UA_MethodAttributes node_attr = UA_MethodAttributes_default;
    node_attr.description =
        UA_LOCALIZEDTEXT_ALLOC("en-US", meta_info->description().c_str());
    node_attr.displayName =
        UA_LOCALIZEDTEXT_ALLOC("en-US", meta_info->name().c_str());
    node_attr.executable = true;
    node_attr.userExecutable = true;

    status = UA_Server_addMethodNode(server_->getServer(), method_node_id,
        parent_id, reference_type_id, method_browse_name, node_attr,
        &NodeCallbackHandler::callNodeMethod, arg_count, input_args,
        output_count, output, nullptr, nullptr);
    checkStatusCode("While adding method node to server", status);
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to create a MethodNode for callable: {}. Status: {}",
        meta_info->name(), ex.what());
  }

  // clean local pointer instances to avoid memory leaks
  if (input_args != nullptr) {
    // input args are copied into method node or are not used in case of failure
    free(input_args);
  }
  if (output != nullptr) {
    // output is copied into method node or not used in case of failure
    UA_Argument_delete(output);
  }
  return status;
}