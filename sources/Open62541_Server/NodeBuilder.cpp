#include "NodeBuilder.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"
#include "Variant_Visitor.hpp"

#include <open62541/nodeids.h>
#include <open62541/statuscodes.h>
#include <string>
#include <vector>

using namespace std;
using namespace HaSLI;
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
    const NonemptyNamedElementPtr& element,
    optional<UA_NodeId> parent_node_id) {
  bool is_root = !parent_node_id.has_value();

  logger_->info("Adding a new node: {}, with id: {}", element->getElementName(),
      element->getElementId());

  auto node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), element->getElementId().c_str());
  logger_->trace("Assigning {} NodeId to element: {}, with id: {}",
      toString(&node_id), element->getElementName(), element->getElementId());

  auto reference_type_id = UA_NODEID_NUMERIC(
      0, (is_root ? UA_NS0ID_ORGANIZES : UA_NS0ID_HASCOMPONENT));

  // if no parent has been provided, set to root objects folder
  if (is_root) {
    parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  }

  auto browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), element->getElementName().c_str());
  logger_->trace("Assigning browse name: {} to element: {}, with id: {}",
      toString(&browse_name), element->getElementName(),
      element->getElementId());

  auto type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);

  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;
  node_attr.description =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->getElementDescription().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->getElementName().c_str());

  auto status = UA_Server_addObjectNode(server_->getServer(), node_id,
      parent_node_id.value(), reference_type_id, browse_name, type_definition,
      node_attr, nullptr, nullptr);

  UA_QualifiedName_clear(&browse_name);
  UA_ObjectAttributes_clear(&node_attr);

  return make_pair(status, node_id);
}

UA_StatusCode NodeBuilder::addDeviceNode(const NonemptyDevicePtr& device) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  auto result = addObjectNode(device);
  status = result.first;

  try {
    checkStatusCode("While creating Object Node for " + device->getElementId() +
            " " + device->getElementName() + " device",
        status);
    auto device_element_group = device->getDeviceElementGroup();
    for (const auto& device_element : device_element_group->getSubelements()) {
      status = addDeviceNodeElement(device_element, result.second);
      checkStatusCode("While adding DeviceElement " +
              device_element->getElementId() + " " +
              device_element->getElementName() + " node",
          status);
    }
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to create a Node for Device: {}. Status: {}",
        device->getElementName(), ex.what());
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

  removeDataSources(&device_node_id);

  auto ret = UA_Server_deleteNode(server_->getServer(), device_node_id, true);

  UA_NodeId_clear(&device_node_id);
  return ret;
}

UA_StatusCode NodeBuilder::addDeviceNodeElement(
    const NonemptyDeviceElementPtr& element, const UA_NodeId& parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->info("Adding element {} to node {}", element->getElementName(),
      toString(&parent_id));

  match(
      element->functionality,
      [&](const NonemptyDeviceElementGroupPtr& group) {
        status = addGroupNode(element, group, parent_id);
      },
      [&](const NonemptyMetricPtr& metric) {
        status = addReadableNode(element, metric, parent_id);
      },
      [&](const NonemptyWritableMetricPtr& metric) {
        status = addWritableNode(element, metric, parent_id);
      },
      [&](const NonemptyFunctionPtr& function) {
        status = addFunctionNode(element, function, parent_id);
      });

  if (status != UA_STATUSCODE_GOOD) {
    logger_->error("Failed to create a Node for Device Element: {}. Status: {}",
        element->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::addGroupNode(
    const NonemptyNamedElementPtr& meta_info,
    const NonemptyDeviceElementGroupPtr& device_element_group,
    const UA_NodeId& parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  if (!device_element_group->getSubelements().empty()) {
    auto result = addObjectNode(meta_info, parent_id);
    status = result.first;
    try {
      checkStatusCode("Parent's " + toString(&parent_id) + " group element " +
              meta_info->getElementName() + " with id " +
              meta_info->getElementId() + " is empty.",
          status);
      auto elements = device_element_group->getSubelements();

      logger_->info("Group element {}:{} contains {} subelements.",
          meta_info->getElementName(), toString(&result.second),
          elements.size());
      for (const auto& element : elements) {
        status = addDeviceNodeElement(element, result.second);
      }
    } catch (const StatusCodeNotGood& ex) {
      logger_->error(
          "Failed to create a Node for Device Element Group: {}. Status: {}",
          meta_info->getElementName(), ex.what());
    }
    UA_NodeId_clear(&result.second);
  }
  return status;
}

template <class MetricType>
UA_StatusCode NodeBuilder::setValue(UA_VariableAttributes& value_attribute,
    const Information_Model::NonemptyNamedElementPtr& meta_info,
    const NonemptyPointer::NonemptyPtr<shared_ptr<MetricType>>& metric,
    const string& metric_type_description) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  try {
    auto variant = metric->getMetricValue();
    value_attribute.value = toUAVariant(variant);
    value_attribute.description = UA_LOCALIZEDTEXT_ALLOC(
        "EN_US", meta_info->getElementDescription().c_str());
    value_attribute.displayName =
        UA_LOCALIZEDTEXT_ALLOC("EN_US", meta_info->getElementName().c_str());
    value_attribute.dataType = toNodeId(metric->getDataType());
    status = UA_STATUSCODE_GOOD;
  } catch (const exception& ex) {
    logger_->error("An exception occurred while trying to set " +
            metric_type_description + " value! Exception: {}",
        ex.what());
  }

  return status;
}

UA_StatusCode NodeBuilder::addReadableNode(
    const NonemptyNamedElementPtr& meta_info, const NonemptyMetricPtr& metric,
    const UA_NodeId& parent_id) {
  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->getElementId().c_str());
  logger_->trace("Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->getElementName().c_str());
  logger_->trace("Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  auto status = setValue(node_attr, meta_info, metric, "readable metric");
  try {
    checkStatusCode("While setting default readable metric value", status);
    logger_->trace("Assigning {} read callback for {} node",
        toString(metric->getDataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef UA_ENABLE_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif // UA_ENABLE_HISTORIZING
    status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
        make_shared<CallbackWrapper>(metric->getDataType(),
            (CallbackWrapper::ReadCallback)bind(
                &Metric::getMetricValue, metric.base())));
    checkStatusCode("While setting readable metric callbacks", status);

    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;
    data_source.write = nullptr;

    auto* server_ptr = server_->getServer();
    status = UA_Server_addDataSourceVariableNode(server_ptr, metrid_node_id,
        parent_id, reference_type_id, metric_browse_name, type_definition,
        node_attr, data_source, nullptr, nullptr);
    checkStatusCode("While adding readable variable node to server", status);
#ifdef UA_ENABLE_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif // UA_ENABLE_HISTORIZING
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Readable Metric: {}. Status: {}",
        meta_info->getElementName(), ex.what());
  }
  UA_NodeId_clear(&metrid_node_id);
  UA_QualifiedName_clear(&metric_browse_name);
  UA_VariableAttributes_clear(&node_attr);
  return status;
}

UA_StatusCode NodeBuilder::addWritableNode(
    const NonemptyNamedElementPtr& meta_info,
    const NonemptyWritableMetricPtr& metric, const UA_NodeId& parent_id) {
  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->getElementId().c_str());
  logger_->trace("Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->getElementName().c_str());
  logger_->trace("Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  auto status = setValue(node_attr, meta_info, metric, "writable metric");
  try {
    checkStatusCode("While setting default writable metric value", status);
    logger_->trace("Assigning {} read and write callbacks for {} node",
        toString(metric->getDataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
#ifdef UA_ENABLE_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif // UA_ENABLE_HISTORIZING
    UA_DataSource data_source;
    if (!metric->isWriteOnly()) {
      status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
          make_shared<CallbackWrapper>(metric->getDataType(),
              (CallbackWrapper::ReadCallback)bind(
                  &WritableMetric::getMetricValue, metric.base()),
              (CallbackWrapper::WriteCallback)bind(
                  &WritableMetric::setMetricValue, metric.base(),
                  placeholders::_1)));
      data_source.read = &NodeCallbackHandler::readNodeValue;
    } else {
      status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
          make_shared<CallbackWrapper>(metric->getDataType(),
              (CallbackWrapper::WriteCallback)bind(
                  &WritableMetric::setMetricValue, metric.base(),
                  placeholders::_1)));
      data_source.read = nullptr;
    }
    checkStatusCode("While setting writable metric callbacks", status);
    data_source.write = &NodeCallbackHandler::writeNodeValue;

    status = UA_Server_addDataSourceVariableNode(server_->getServer(),
        metrid_node_id, parent_id, reference_type_id, metric_browse_name,
        type_definition, node_attr, data_source, nullptr, nullptr);
    checkStatusCode("While adding writable variable node to server", status);
#ifdef UA_ENABLE_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif // UA_ENABLE_HISTORIZING
  } catch (const StatusCodeNotGood& ex) {
    logger_->error(
        "Failed to create a Node for Writable Metric: {}. Status: {}",
        meta_info->getElementName(), ex.what());
  }
  UA_NodeId_clear(&metrid_node_id);
  UA_QualifiedName_clear(&metric_browse_name);
  UA_VariableAttributes_clear(&node_attr);
  return status;
}

UA_StatusCode NodeBuilder::addFunctionNode(
    const NonemptyNamedElementPtr& meta_info,
    const NonemptyFunctionPtr& function, const UA_NodeId& parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  auto method_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->getElementId().c_str());
  if (function->result_type != DataType::NONE ||
      function->result_type != DataType::UNKNOWN) {
    CallbackWrapper::CallCallback call_cb =
        bind(static_cast<DataVariant (Function::*)(
                 const Function::Parameters&, uintmax_t)>(&Function::call),
            function.base(), placeholders::_1,
            (uintmax_t)1000); // NOLINT(readability-magic-numbers) call-timeout
    status = NodeCallbackHandler::addNodeCallbacks(method_node_id,
        make_shared<CallbackWrapper>(
            function->result_type, function->parameters, move(call_cb)));
  } else {
    CallbackWrapper::ExecuteCallback execute_cb =
        bind(&Function::execute, function.base(), placeholders::_1);
    status = NodeCallbackHandler::addNodeCallbacks(method_node_id,
        make_shared<CallbackWrapper>(
            function->result_type, function->parameters, move(execute_cb)));
  }
  UA_Argument* input_args = nullptr;
  UA_Argument* output = nullptr;
  try {
    checkStatusCode("While setting function callbacks", status);
    auto arg_count = function->parameters.size();
    if (arg_count > 0) {
      input_args = (UA_Argument*)malloc(arg_count * sizeof(UA_Argument));
      for (size_t i = 0; i < function->parameters.size(); ++i) {
        UA_Argument_init(&input_args[i]);
        auto parameter = function->parameters.at(i);
        input_args[i].name = makeUAString(toString(parameter.first));
        input_args[i].dataType = toNodeId(parameter.first);
        input_args[i].valueRank =
            UA_VALUERANK_SCALAR; // each input type is scalar
        auto arg_desc =
            (parameter.second ? "Optional " : "") + toString(parameter.first);
        input_args[i].description =
            UA_LOCALIZEDTEXT_ALLOC("EN_US", arg_desc.c_str());
      }
      uint8_t output_count = 0;
      if (function->result_type != DataType::NONE) {
        output_count = 1;
        output = UA_Argument_new();
        output->name = makeUAString(toString(function->result_type));
        output->dataType = toNodeId(function->result_type);
        output->valueRank = UA_VALUERANK_SCALAR; // all returns are scalar
        auto ouput_desc = toString(function->result_type);
        output->description =
            UA_LOCALIZEDTEXT_ALLOC("EN_US", ouput_desc.c_str());
      }
      auto reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
      auto method_browse_name = UA_QUALIFIEDNAME_ALLOC(
          server_->getServerNamespace(), meta_info->getElementName().c_str());

      UA_MethodAttributes node_attr = UA_MethodAttributes_default;
      node_attr.description = UA_LOCALIZEDTEXT_ALLOC(
          "en-US", meta_info->getElementDescription().c_str());
      node_attr.displayName =
          UA_LOCALIZEDTEXT_ALLOC("en-US", meta_info->getElementName().c_str());
      node_attr.executable = true;
      node_attr.userExecutable = true;

      status = UA_Server_addMethodNode(server_->getServer(), method_node_id,
          parent_id, reference_type_id, method_browse_name, node_attr,
          &NodeCallbackHandler::callNodeMethod, arg_count, input_args,
          output_count, output, nullptr, nullptr);
      checkStatusCode("While adding method node to server", status);
    }
  } catch (const StatusCodeNotGood& ex) {
    logger_->error("Failed to create a MethodNode for Function: {}. Status: {}",
        meta_info->getElementName(), ex.what());
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