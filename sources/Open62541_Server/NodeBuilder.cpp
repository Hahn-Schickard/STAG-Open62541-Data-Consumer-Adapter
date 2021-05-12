#include "NodeBuilder.hpp"
#include "LoggerRepository.hpp"
#include "Utility.hpp"
#include "Variant_Visitor.hpp"

#include <open62541/nodeids.h>
#include <open62541/statuscodes.h>
#include <string>
#include <vector>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace open62541;

NodeBuilder::NodeBuilder(shared_ptr<Open62541Server> server)
    : logger_(LoggerRepository::getInstance().registerTypedLoger(this)),
      server_(server) {
  NodeCallbackHandler::initialise(server->getServerLogger());
}

NodeBuilder::~NodeBuilder() {
  logger_->log(SeverityLevel::INFO, "Removing {} from logger registry",
               logger_->getName());
  NodeCallbackHandler::destroy();
  LoggerRepository::getInstance().deregisterLoger(logger_->getName());
}

UA_StatusCode NodeBuilder::addDeviceNode(shared_ptr<Device> device) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->log(SeverityLevel::INFO, "Adding a new Device: {}, with id: {}",
               device->getElementName(), device->getElementId());

  UA_NodeId device_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), device->getElementId().c_str());
  logger_->log(SeverityLevel::TRACE,
               "Assigning {} NodeId to Device: {}, with id: {}",
               toString(&device_node_id), device->getElementName(),
               device->getElementId());
  UA_NodeId parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName device_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), device->getElementName().c_str());
  logger_->log(SeverityLevel::TRACE,
               "Assigning browse name: {} to Device: {}, with id: {}",
               toString(&device_browse_name), device->getElementName(),
               device->getElementId());
  UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

  node_attr.description =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", device->getElementDescription().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT_ALLOC("EN_US", device->getElementName().c_str());

  status = UA_Server_addObjectNode(
      server_->getServer(), device_node_id, parent_node_id, reference_type_id,
      device_browse_name, type_definition, node_attr, NULL, NULL);

  if (status == UA_STATUSCODE_GOOD) {
    shared_ptr<DeviceElementGroup> device_element_group =
        device->getDeviceElementGroup();
    for (auto device_element : device_element_group->getSubelements()) {
      status = addDeviceNodeElement(device_element, &device_node_id);
    }
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
                 "Failed to create a Node for Device: {}. Status: {}",
                 device->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode
NodeBuilder::addDeviceNodeElement(shared_ptr<DeviceElement> element,
                                  const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->log(SeverityLevel::INFO, "Adding element {} to node {}",
               element->getElementName(), toString(parent_id));

  switch (element->getElementType()) {
  case GROUP: {
    status = addGroupNode(static_pointer_cast<DeviceElementGroup>(element),
                          parent_id);
    break;
  }
  case FUNCTION: {
    status = addFunctionNode(element, parent_id);
    break;
  }
  case OBSERVABLE:
  case WRITABLE: {
    status = addWritableNode(static_pointer_cast<WritableMetric>(element),
                             parent_id);
    break;
  }
  case READABLE: {
    status = addReadableNode(static_pointer_cast<Metric>(element), parent_id);
    break;
  }
  case UNDEFINED:
  default: {
    status = UA_STATUSCODE_BADTYPEDEFINITIONINVALID;
    logger_->log(SeverityLevel::ERROR,
                 "Unknown element type, for element {} to node {}",
                 element->getElementName(), toString(parent_id));
    break;
  }
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
                 "Failed to create a Node for Device Element: {}. Status: {}",
                 element->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode
NodeBuilder::addGroupNode(shared_ptr<DeviceElementGroup> device_element_group,
                          const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  if (!device_element_group->getSubelements().empty()) {
    logger_->log(SeverityLevel::TRACE,
                 "Adding group element {} with ID {} to node {}",
                 device_element_group->getElementName(),
                 device_element_group->getElementId(), toString(parent_id));

    UA_NodeId group_node_id =
        UA_NODEID_STRING_ALLOC(server_->getServerNamespace(),
                               device_element_group->getElementId().c_str());
    logger_->log(SeverityLevel::TRACE,
                 "Assigning {} NodeId to group element: {}, with id: {}",
                 toString(&group_node_id),
                 device_element_group->getElementName(),
                 device_element_group->getElementId());
    UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_QualifiedName group_browse_name =
        UA_QUALIFIEDNAME_ALLOC(server_->getServerNamespace(),
                               device_element_group->getElementName().c_str());
    logger_->log(SeverityLevel::TRACE,
                 "Assigning browse name: {} to group element: {}, with id: {}",
                 toString(&group_browse_name),
                 device_element_group->getElementName(),
                 device_element_group->getElementId());
    UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

    node_attr.description = UA_LOCALIZEDTEXT_ALLOC(
        "EN_US", device_element_group->getElementDescription().c_str());
    node_attr.displayName = UA_LOCALIZEDTEXT_ALLOC(
        "EN_US", device_element_group->getElementName().c_str());

    status = UA_Server_addObjectNode(
        server_->getServer(), group_node_id, *parent_id, reference_type_id,
        group_browse_name, type_definition, node_attr, NULL, NULL);

    if (status == UA_STATUSCODE_GOOD) {
      vector<shared_ptr<DeviceElement>> elements =
          device_element_group->getSubelements();

      logger_->log(SeverityLevel::INFO,
                   "Group element {}:{} contains {} subelements.",
                   device_element_group->getElementName(),
                   toString(&group_node_id), elements.size());
      for (auto element : elements) {
        status = addDeviceNodeElement(element, &group_node_id);
      }
    }
  } else {
    logger_->log(SeverityLevel::WARNNING,
                 "Parent's {} group element {} with id {} is empty!",
                 toString(parent_id), device_element_group->getElementName(),
                 device_element_group->getElementId());
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(
        SeverityLevel::ERROR,
        "Failed to create a Node for Device Element Group: {}. Status: {}",
        device_element_group->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::addFunctionNode(shared_ptr<DeviceElement> function,
                                           const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTIMPLEMENTED;
  logger_->log(SeverityLevel::WARNNING, "Method element is not implemented!",
               toString(parent_id), function->getElementName());
  //@TODO: Implement addFunctionNode stub
  return status;
}

void setVariant(UA_VariableAttributes &value_attribute, DataVariant variant) {
  match(variant,
        [&](bool value) {
          UA_Variant_setScalar(&value_attribute.value, &value,
                               &UA_TYPES[UA_TYPES_BOOLEAN]);
        },
        [&](uint64_t value) {
          UA_Variant_setScalar(&value_attribute.value, &value,
                               &UA_TYPES[UA_TYPES_UINT64]);
        },
        [&](int64_t value) {
          UA_Variant_setScalar(&value_attribute.value, &value,
                               &UA_TYPES[UA_TYPES_INT64]);
        },
        [&](double value) {
          UA_Variant_setScalar(&value_attribute.value, &value,
                               &UA_TYPES[UA_TYPES_DOUBLE]);
        },
        [&](DateTime value) {
          auto date_time = UA_DateTime_toStruct(value.getValue());
          UA_Variant_setScalar(&value_attribute.value, &date_time,
                               &UA_TYPES[UA_TYPES_DATETIME]);
        },
        [&](vector<uint8_t> value) {
          string tmp(value.begin(), value.end());
          auto byte_string = UA_BYTESTRING_ALLOC(tmp.c_str());
          UA_Variant_setScalar(&value_attribute.value, &byte_string,
                               &UA_TYPES[UA_TYPES_BYTESTRING]);
        },
        [&](string value) {
          UA_String open62541_string;
          open62541_string.length = strlen(value.c_str());
          open62541_string.data = (UA_Byte *)malloc(open62541_string.length);
          memcpy(open62541_string.data, value.c_str(), open62541_string.length);
          UA_Variant_setScalar(&value_attribute.value, &open62541_string,
                               &UA_TYPES[UA_TYPES_STRING]);
        });
}

UA_StatusCode NodeBuilder::setValue(UA_VariableAttributes &value_attribute,
                                    MetricPtr metric) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  if (metric) {
    try {
      auto variant = metric->getMetricValue();
      setVariant(value_attribute, variant);
      if (!UA_Variant_isEmpty(&value_attribute.value)) {
        value_attribute.description = UA_LOCALIZEDTEXT_ALLOC(
            "EN_US", metric->getElementDescription().c_str());
        value_attribute.displayName =
            UA_LOCALIZEDTEXT_ALLOC("EN_US", metric->getElementName().c_str());
        value_attribute.dataType = toNodeId(metric->getDataType());
        status = UA_STATUSCODE_GOOD;
      }
    } catch (exception &ex) {
      logger_->log(
          SeverityLevel::ERROR,
          "An exception occurred while trying to set readable metric value! "
          "Exception: {}",
          ex.what());
    }
  } else {
    logger_->log(SeverityLevel::ERROR,
                 "Can not set a value for non existant readable metric!");
  }
  return status;
}

UA_StatusCode NodeBuilder::addReadableNode(MetricPtr metric,
                                           const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), metric->getElementId().c_str());
  logger_->log(SeverityLevel::TRACE,
               "Assigning {} NodeId to metric: {}, with id: {}",
               toString(&metrid_node_id), metric->getElementName(),
               metric->getElementId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), metric->getElementName().c_str());
  logger_->log(SeverityLevel::TRACE,
               "Assigning browse name: {} to metric: {}, with id: {}",
               toString(&metric_browse_name), metric->getElementName(),
               metric->getElementId());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  status = setValue(node_attr, metric);

  if (status == UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::TRACE, "Assigning {} read callback for {} node",
                 toString(metric->getDataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    status = NodeCallbackHandler::addNodeCallbacks(
        metrid_node_id,
        make_shared<CallbackWrapper>(metric->getDataType(),
                                     bind(&Metric::getMetricValue, metric)));
    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;

    auto server_ptr = server_->getServer();
    status = UA_Server_addDataSourceVariableNode(
        server_ptr, metrid_node_id, *parent_id, reference_type_id,
        metric_browse_name, type_definition, node_attr, data_source, NULL,
        NULL);
  }
  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
                 "Failed to create a Node for Readable Metric: {}. Status: {}",
                 metric->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}

UA_StatusCode NodeBuilder::setValue(UA_VariableAttributes &value_attribute,
                                    WritableMetricPtr metric) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  if (metric) {
    try {
      auto variant = metric->getMetricValue();
      setVariant(value_attribute, variant);

      value_attribute.description = UA_LOCALIZEDTEXT_ALLOC(
          "EN_US", metric->getElementDescription().c_str());
      value_attribute.displayName =
          UA_LOCALIZEDTEXT_ALLOC("EN_US", metric->getElementName().c_str());

      value_attribute.dataType = toNodeId(metric->getDataType());
    } catch (exception &ex) {
      logger_->log(
          SeverityLevel::ERROR,
          "An exception occurred while trying to set writable metric value! "
          "Exception: {}",
          ex.what());
    }
  } else {
    logger_->log(SeverityLevel::ERROR,
                 "Can not set a value for non existant writable metric!");
  }

  return status;
}

UA_StatusCode NodeBuilder::addWritableNode(WritableMetricPtr metric,
                                           const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  UA_NodeId metrid_node_id = UA_NODEID_STRING_ALLOC(
      server_->getServerNamespace(), metric->getElementId().c_str());
  logger_->log(SeverityLevel::TRACE,
               "Assigning {} NodeId to metric: {}, with id: {}",
               toString(&metrid_node_id), metric->getElementName(),
               metric->getElementId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME_ALLOC(
      server_->getServerNamespace(), metric->getElementName().c_str());
  logger_->log(SeverityLevel::TRACE,
               "Assigning browse name: {} to metric: {}, with id: {}",
               toString(&metric_browse_name), metric->getElementName(),
               metric->getElementId());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  status = setValue(node_attr, metric);

  if (status == UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::TRACE,
                 "Assigning {} read and write callbacks for {} node",
                 toString(metric->getDataType()), toString(&metrid_node_id));
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    status = NodeCallbackHandler::addNodeCallbacks(
        metrid_node_id,
        make_shared<CallbackWrapper>(
            metric->getDataType(),
            bind(&WritableMetric::getMetricValue, metric),
            bind(&WritableMetric::setMetricValue, metric, placeholders::_1)));
    UA_DataSource data_source;
    data_source.read = &NodeCallbackHandler::readNodeValue;
    data_source.write = &NodeCallbackHandler::writeNodeValue;

    status = UA_Server_addDataSourceVariableNode(
        server_->getServer(), metrid_node_id, *parent_id, reference_type_id,
        metric_browse_name, type_definition, node_attr, data_source, NULL,
        NULL);
  }

  if (status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
                 "Failed to create a Node for Writable Metric: {}. Status: {}",
                 metric->getElementName(), UA_StatusCode_name(status));
  }

  return status;
}