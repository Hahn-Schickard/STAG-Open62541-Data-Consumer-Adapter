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

NodeBuilder::NodeBuilder(shared_ptr<Open62541Server> server)
    : logger_(LoggerManager::registerTypedLogger(this)), server_(server) {
  NodeCallbackHandler::initialise(server->getServerLogger());
}

NodeBuilder::~NodeBuilder() { cleanup(); }

void NodeBuilder::cleanup() { NodeCallbackHandler::destroy(); }

pair<UA_StatusCode, UA_NodeId> NodeBuilder::addObjectNode(
    NonemptyNamedElementPtr element, optional<UA_NodeId> parent_node_id) {
  bool is_root = !parent_node_id.has_value();

  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->log(SeverityLevel::INFO, "Adding a new node: {}, with id: {}",
      element->getElementName(), element->getElementId());

  auto node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), element->getElementId().c_str());
  logger_->log(SeverityLevel::TRACE,
      "Assigning {} NodeId to element: {}, with id: {}", toString(&node_id),
      element->getElementName(), element->getElementId());

  auto reference_type_id = UA_NODEID_NUMERIC(
      0, (is_root ? UA_NS0ID_ORGANIZES : UA_NS0ID_HASCOMPONENT));

  // if no parent has been provided, set to root objects folder
  if (is_root) {
    parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  }

  auto browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), element->getElementName().c_str());
  logger_->log(SeverityLevel::TRACE,
      "Assigning browse name: {} to element: {}, with id: {}",
      toString(&browse_name), element->getElementName(),
      element->getElementId());

  auto type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);

  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;
  node_attr.description =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->getElementDescription().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", element->getElementName().c_str());

  status = UA_Server_addObjectNode(server_->getServer(), node_id,
      parent_node_id.value(), reference_type_id, browse_name, type_definition,
      node_attr, nullptr, nullptr);
  return make_pair(status, node_id);
}

UA_StatusCode NodeBuilder::addDeviceNode(NonemptyDevicePtr device) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  auto result = addObjectNode(device);
  status = result.first;

  if (status == UA_STATUSCODE_GOOD) {
    auto device_element_group = device->getDeviceElementGroup();
    for (auto device_element : device_element_group->getSubelements()) {
      status = addDeviceNodeElement(device_element, result.second);
    }
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "Failed to create a Node for Device: {}. Status: {}",
        device->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::deleteDeviceNode(const string& device_id) {
  auto node_id =
      UA_NODEID_STRING_ALLOC(server_->getServerNamespace(), device_id.c_str());
  logger_->log(SeverityLevel::TRACE, "Removing Node {}", toString(&node_id));
  return UA_Server_deleteNode(server_->getServer(), node_id, true);
}

UA_StatusCode NodeBuilder::addDeviceNodeElement(
    NonemptyDeviceElementPtr element, UA_NodeId parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->log(SeverityLevel::INFO, "Adding element {} to node {}",
      element->getElementName(), toString(&parent_id));

  match(element->specific_interface,
      [&](NonemptyDeviceElementGroupPtr group) {
        status = addGroupNode(element, group, parent_id);
      },
      [&](NonemptyMetricPtr metric) {
        status = addReadableNode(element, metric, parent_id);
      },
      [&](NonemptyWritableMetricPtr metric) {
        status = addWritableNode(element, metric, parent_id);
      });

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "Failed to create a Node for Device Element: {}. Status: {}",
        element->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::addGroupNode(NonemptyNamedElementPtr meta_info,
    NonemptyDeviceElementGroupPtr device_element_group, UA_NodeId parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  if (!device_element_group->getSubelements().empty()) {
    auto result = addObjectNode(meta_info, parent_id);
    status = result.first;

    if (status == UA_STATUSCODE_GOOD) {
      auto elements = device_element_group->getSubelements();

      logger_->log(SeverityLevel::INFO,
          "Group element {}:{} contains {} subelements.",
          meta_info->getElementName(), toString(&result.second),
          elements.size());
      for (auto element : elements) {
        status = addDeviceNodeElement(element, result.second);
      }
    }
  } else {
    logger_->log(SeverityLevel::WARNING,
        "Parent's {} group element {} with id {} is empty!",
        toString(&parent_id), meta_info->getElementName(),
        meta_info->getElementId());
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "Failed to create a Node for Device Element Group: {}. Status: {}",
        meta_info->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::addFunctionNode(
    DeviceElementPtr function, UA_NodeId parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTIMPLEMENTED;
  logger_->log(SeverityLevel::WARNING, "Method element is not implemented!",
      toString(&parent_id), function->getElementName());
  //@TODO: Implement addFunctionNode stub
  return status;
}

void setVariant(UA_VariableAttributes& value_attribute, DataVariant variant) {
  // Postcondition: value_attribute.value is non-empty
  match(variant,
      [&](bool value) {
        UA_Variant_setScalarCopy(
            &value_attribute.value, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
      },
      [&](uint64_t value) {
        UA_Variant_setScalarCopy(
            &value_attribute.value, &value, &UA_TYPES[UA_TYPES_UINT64]);
      },
      [&](int64_t value) {
        UA_Variant_setScalarCopy(
            &value_attribute.value, &value, &UA_TYPES[UA_TYPES_INT64]);
      },
      [&](double value) {
        UA_Variant_setScalarCopy(
            &value_attribute.value, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
      },
      [&](DateTime value) {
        auto date_time = UA_DateTime_toStruct(value.getValue());
        UA_Variant_setScalarCopy(
            &value_attribute.value, &date_time, &UA_TYPES[UA_TYPES_DATETIME]);
      },
      [&](vector<uint8_t> value) {
        string tmp(value.begin(), value.end());
        auto byte_string = UA_BYTESTRING_ALLOC(tmp.c_str());
        UA_Variant_setScalarCopy(&value_attribute.value, &byte_string,
            &UA_TYPES[UA_TYPES_BYTESTRING]);
      },
      [&](const string& value) {
        UA_String open62541_string;
        open62541_string.length = strlen(value.c_str());
        open62541_string.data = (UA_Byte*)malloc(open62541_string.length);
        memcpy(open62541_string.data, value.c_str(), open62541_string.length);
        UA_Variant_setScalarCopy(&value_attribute.value, &open62541_string,
            &UA_TYPES[UA_TYPES_STRING]);
      });
}

template <class MetricType>
UA_StatusCode NodeBuilder::setValue(UA_VariableAttributes& value_attribute,
    Information_Model::NonemptyNamedElementPtr meta_info,
    NonemptyPointer::NonemptyPtr<std::shared_ptr<MetricType>> metric,
    std::string metric_type_description) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  try {
    auto variant = metric->getMetricValue();
    setVariant(value_attribute, variant);
    value_attribute.description = UA_LOCALIZEDTEXT_ALLOC(
        "EN_US", meta_info->getElementDescription().c_str());
    value_attribute.displayName =
        UA_LOCALIZEDTEXT_ALLOC("EN_US", meta_info->getElementName().c_str());
    value_attribute.dataType = toNodeId(metric->getDataType());
    status = UA_STATUSCODE_GOOD;
  } catch (exception& ex) {
    logger_->log(SeverityLevel::ERROR,
        "An exception occurred while trying to set " + metric_type_description +
            " value! "
            "Exception: {}",
        ex.what());
  }

  return status;
}

UA_StatusCode NodeBuilder::addReadableNode(NonemptyNamedElementPtr meta_info,
    NonemptyMetricPtr metric, UA_NodeId parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->getElementId().c_str());
  logger_->log(SeverityLevel::TRACE,
      "Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->getElementName().c_str());
  logger_->log(SeverityLevel::TRACE,
      "Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  status = setValue(node_attr, meta_info, metric, "readable metric");

  if (status == UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::TRACE, "Assigning {} read callback for {} node",
        toString(metric->getDataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef UA_ENABLE_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif
    status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
        make_shared<CallbackWrapper>(metric->getDataType(),
            bind(&Metric::getMetricValue, metric.base())));
    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;

    auto* server_ptr = server_->getServer();
    status = UA_Server_addDataSourceVariableNode(server_ptr, metrid_node_id,
        parent_id, reference_type_id, metric_browse_name, type_definition,
        node_attr, data_source, nullptr, nullptr);
#ifdef UA_ENABLE_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif
  }
  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "Failed to create a Node for Readable Metric: {}. Status: {}",
        meta_info->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::addWritableNode(NonemptyNamedElementPtr meta_info,
    NonemptyWritableMetricPtr metric, UA_NodeId parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), meta_info->getElementId().c_str());
  logger_->log(SeverityLevel::TRACE,
      "Assigning {} NodeId to metric: {}, with id: {}",
      toString(&metrid_node_id), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), meta_info->getElementName().c_str());
  logger_->log(SeverityLevel::TRACE,
      "Assigning browse name: {} to metric: {}, with id: {}",
      toString(&metric_browse_name), meta_info->getElementName(),
      meta_info->getElementId());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  status = setValue(node_attr, meta_info, metric, "writable metric");

  if (status == UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::TRACE,
        "Assigning {} read and write callbacks for {} node",
        toString(metric->getDataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
#ifdef UA_ENABLE_HISTORIZING
    node_attr.accessLevel |= UA_ACCESSLEVELMASK_HISTORYREAD;
    node_attr.historizing = true;
#endif
    status = NodeCallbackHandler::addNodeCallbacks(metrid_node_id,
        make_shared<CallbackWrapper>(metric->getDataType(),
            bind(&WritableMetric::getMetricValue, metric.base()),
            bind(&WritableMetric::setMetricValue, metric.base(),
                placeholders::_1)));
    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;
    data_source.write = &NodeCallbackHandler::writeNodeValue;

    status = UA_Server_addDataSourceVariableNode(server_->getServer(),
        metrid_node_id, parent_id, reference_type_id, metric_browse_name,
        type_definition, node_attr, data_source, nullptr, nullptr);
#ifdef UA_ENABLE_HISTORIZING
    server_->registerForHistorization(metrid_node_id, node_attr.value.type);
#endif
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "Failed to create a Node for Writable Metric: {}. Status: {}",
        meta_info->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}