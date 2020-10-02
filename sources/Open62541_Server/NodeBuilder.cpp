#include "NodeBuilder.hpp"
#include "DataVariant.hpp"
#include "LoggerRepository.hpp"
#include "NodeMananger.hpp"
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

class DeviceElementNodeInfo {
  char *node_id_;
  char *node_name_;
  char *node_description_;

public:
  DeviceElementNodeInfo(shared_ptr<NamedElement> element) {
    if (element) {
      node_id_ = new char[element->getElementId().length() + 1];
      node_name_ = new char[element->getElementDescription().length() + 1];
      node_description_ =
          new char[element->getElementDescription().length() + 1];

      strcpy(node_id_, element->getElementId().c_str());
      strcpy(node_name_, element->getElementName().c_str());
      strcpy(node_description_, element->getElementDescription().c_str());
    } else {
      throw runtime_error("Given element is empty!");
    }
  }

  ~DeviceElementNodeInfo() {
    delete[] node_id_;
    delete[] node_name_;
    delete[] node_description_;
  }

  char *getNodeId() { return node_id_; }
  char *getNodeName() { return node_name_; }
  char *getNodeDescription() { return node_description_; }
};

NodeBuilder::NodeBuilder(Open62541Server *server)
    : logger_(LoggerRepository::getInstance().registerTypedLoger(this)),
      manager_(make_unique<NodeManager>()), server_(server) {}

NodeBuilder::~NodeBuilder() {
  logger_->log(SeverityLevel::INFO, "Removing {} from logger registery",
               logger_->getName());
  LoggerRepository::getInstance().deregisterLoger(logger_->getName());
}

UA_StatusCode NodeBuilder::addDeviceNode(shared_ptr<Device> device) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;
  logger_->log(SeverityLevel::INFO, "Adding a new Device: {}, with id: {}",
               device->getElementName(), device->getElementId());

  DeviceElementNodeInfo device_node_info = DeviceElementNodeInfo(device);

  UA_NodeId device_node_id = UA_NODEID_STRING(server_->getServerNamespace(),
                                              device_node_info.getNodeId());
  UA_NodeId parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName device_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), device_node_info.getNodeName());
  UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

  node_attr.description =
      UA_LOCALIZEDTEXT("EN_US", device_node_info.getNodeDescription());
  node_attr.displayName =
      UA_LOCALIZEDTEXT("EN_US", device_node_info.getNodeName());

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
  logger_->log(SeverityLevel::TRACE, "Adding group element {} to node {}",
               device_element_group->getElementName(), toString(parent_id));

  DeviceElementNodeInfo element_node_info =
      DeviceElementNodeInfo(device_element_group);

  if (!device_element_group->getSubelements().empty()) {
    UA_NodeId group_node_id = UA_NODEID_STRING(server_->getServerNamespace(),
                                               element_node_info.getNodeId());
    UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_QualifiedName group_browse_name = UA_QUALIFIEDNAME(
        server_->getServerNamespace(), element_node_info.getNodeName());
    UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

    node_attr.description =
        UA_LOCALIZEDTEXT("EN_US", element_node_info.getNodeDescription());
    node_attr.displayName =
        UA_LOCALIZEDTEXT("EN_US", element_node_info.getNodeName());

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
                 "Parent's {} group element {} is empty!", toString(parent_id),
                 device_element_group->getElementName());
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
        [&](vector<uint8_t> value) {
          throw runtime_error("Array type is not supported!");
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

UA_StatusCode setValue(DeviceElementNodeInfo *element_node_info,
                       UA_VariableAttributes &value_attribute,
                       shared_ptr<Metric> metric) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  setVariant(value_attribute, metric->getMetricValue());

  value_attribute.description =
      UA_LOCALIZEDTEXT("EN_US", element_node_info->getNodeDescription());
  value_attribute.displayName =
      UA_LOCALIZEDTEXT("EN_US", element_node_info->getNodeName());

  value_attribute.dataType = toNodeId(metric->getDataType());

  return status;
}

typedef UA_StatusCode (*UA_READ_CB)(UA_Server *server,
                                    const UA_NodeId *sessionId,
                                    void *sessionContext,
                                    const UA_NodeId *nodeId, void *nodeContext,
                                    UA_Boolean includeSourceTimeStamp,
                                    const UA_NumericRange *range,
                                    UA_DataValue *value);

UA_StatusCode NodeBuilder::addReadableNode(shared_ptr<Metric> metric,
                                           const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  DeviceElementNodeInfo element_node_info = DeviceElementNodeInfo(metric);

  UA_NodeId metrid_node_id = UA_NODEID_STRING(server_->getServerNamespace(),
                                              element_node_info.getNodeId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), element_node_info.getNodeName());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  status = setValue(&element_node_info, node_attr, metric);

  if (status == UA_STATUSCODE_GOOD) {
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
    status = manager_->addNode(metric->getDataType(), &metrid_node_id,
                               bind(&Metric::getMetricValue, metric));
    UA_DataSource data_source;

    auto read_cb = bind(&NodeManager::readNodeValue, manager_.get(),
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        placeholders::_4, placeholders::_5, placeholders::_6,
                        placeholders::_7, placeholders::_8);
    data_source.read = (UA_READ_CB)&read_cb;

    status = UA_Server_addDataSourceVariableNode(
        server_->getServer(), metrid_node_id, *parent_id, reference_type_id,
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

UA_StatusCode setValue(DeviceElementNodeInfo *element_node_info,
                       UA_VariableAttributes &value_attribute,
                       shared_ptr<WritableMetric> metric) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  setVariant(value_attribute, metric->getMetricValue());

  value_attribute.description =
      UA_LOCALIZEDTEXT("EN_US", element_node_info->getNodeDescription());
  value_attribute.displayName =
      UA_LOCALIZEDTEXT("EN_US", element_node_info->getNodeName());

  value_attribute.dataType = toNodeId(metric->getDataType());

  return status;
}

typedef UA_StatusCode (*UA_WRITE_CB)(UA_Server *server,
                                     const UA_NodeId *sessionId,
                                     void *sessionContext,
                                     const UA_NodeId *nodeId, void *nodeContext,
                                     const UA_NumericRange *range,
                                     const UA_DataValue *value);

UA_StatusCode NodeBuilder::addWritableNode(shared_ptr<WritableMetric> metric,
                                           const UA_NodeId *parent_id) {
  UA_StatusCode status = UA_STATUSCODE_BADINTERNALERROR;

  DeviceElementNodeInfo element_node_info = DeviceElementNodeInfo(metric);

  UA_NodeId metrid_node_id = UA_NODEID_STRING(server_->getServerNamespace(),
                                              element_node_info.getNodeId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), element_node_info.getNodeName());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  status = setValue(&element_node_info, node_attr, metric);

  if (status == UA_STATUSCODE_GOOD) {
    node_attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    status = manager_->addNode(
        metric->getDataType(), &metrid_node_id,
        bind(&WritableMetric::getMetricValue, metric),
        bind(&WritableMetric::setMetricValue, metric, placeholders::_1));
    UA_DataSource data_source;

    auto read_cb = bind(&NodeManager::readNodeValue, manager_.get(),
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        placeholders::_4, placeholders::_5, placeholders::_6,
                        placeholders::_7, placeholders::_8);
    data_source.read = (UA_READ_CB)&read_cb;
    auto write_cb =
        bind(&NodeManager::writeNodeValue, manager_.get(), placeholders::_1,
             placeholders::_2, placeholders::_3, placeholders::_4,
             placeholders::_5, placeholders::_6, placeholders::_7);
    data_source.write = (UA_WRITE_CB)&write_cb;

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