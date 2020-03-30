#include "NodeBuilder.hpp"

#include "LoggerRepository.hpp"

#include <memory>
#include <open62541/nodeids.h>
#include <string>
#include <vector>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;

string uaStringToCppString(UA_String input) {
  string output;
  for (size_t i = 0; i < input.length; i++) {
    output.push_back(*input.data);
  }
  return output;
}

string uaGuIdToCppString(UA_Guid input) {
  string output = to_string(input.data1) + ":" + to_string(input.data2) + ":" +
                  to_string(input.data3) + ":";
  for (int i = 0; i < 8; i++) {
    output.push_back(input.data4[i]);
  }
  return output;
}

string nodeIdToString(UA_NodeId nodeId) {
  string namespace_index_string = to_string(nodeId.namespaceIndex);
  string node_id_string;
  switch (nodeId.identifierType) {
  case UA_NodeIdType::UA_NODEIDTYPE_NUMERIC: {
    node_id_string = to_string(nodeId.identifier.numeric);
    break;
  }
  case UA_NodeIdType::UA_NODEIDTYPE_GUID: {
    node_id_string = uaGuIdToCppString(nodeId.identifier.guid);
    break;
  }
  case UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING: {
  }
  case UA_NodeIdType::UA_NODEIDTYPE_STRING: {
    node_id_string = uaStringToCppString(nodeId.identifier.string);
    break;
  }
  default: { node_id_string = "UNRECOGNIZED"; }
  }
  return namespace_index_string + ":" + node_id_string;
}

class DeviceElementNodeInfo {
  char *node_id_;
  char *node_name_;
  char *node_description_;

public:
  DeviceElementNodeInfo(shared_ptr<NamedElement> element) {
    if (element) {
      node_id_ = new char[element->getElementRefId().length() + 1];
      node_name_ = new char[element->getElementDescription().length() + 1];
      node_description_ =
          new char[element->getElementDescription().length() + 1];

      strcpy(node_id_, element->getElementRefId().c_str());
      strcpy(node_name_, element->getElementName().c_str());
      strcpy(node_description_, element->getElementDescription().c_str());
    } else {
      // @TODO: thorw an exception that will be logged by the builder
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
    : server_(server),
      logger_(LoggerRepository::getInstance().registerTypedLoger(this)) {}

NodeBuilder::~NodeBuilder() {
  logger_->log(SeverityLevel::INFO, "Removing {} from logger registery",
               logger_->getName());
  LoggerRepository::getInstance().deregisterLoger(logger_->getName());
}

bool NodeBuilder::addDeviceNode(shared_ptr<Device> device) {
  logger_->log(SeverityLevel::INFO, "Adding a new Device: {}, with id: {}",
               device->getElementName(), device->getElementRefId());

  DeviceElementNodeInfo device_node_info =
      DeviceElementNodeInfo(static_pointer_cast<NamedElement>(device));

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

  UA_Server_addObjectNode(server_->getServer(), device_node_id, parent_node_id,
                          reference_type_id, device_browse_name,
                          type_definition, node_attr, NULL, NULL);

  shared_ptr<DeviceElementGroup> device_element_group =
      device->getDeviceElementGroup();
  bool status = false;
  for (auto device_element : device_element_group->getSubelements()) {
    status = addDeviceNodeElement(device_element, device_node_id);
  }

  return status;
}

bool NodeBuilder::addDeviceNodeElement(shared_ptr<DeviceElement> element,
                                       UA_NodeId parent_id) {
  logger_->log(SeverityLevel::INFO, "Adding element {} to node {}",
               element->getElementName(), nodeIdToString(parent_id));
  bool status = false;
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
    logger_->log(SeverityLevel::ERROR,
                 "Unknown element type, for element {} to node {}",
                 element->getElementName(), nodeIdToString(parent_id));
    break;
  }
  }
  return status;
}

bool NodeBuilder::addGroupNode(
    shared_ptr<DeviceElementGroup> device_element_group, UA_NodeId parent_id) {
  logger_->log(SeverityLevel::TRACE, "Adding group element {} to node {}",
               device_element_group->getElementName(),
               nodeIdToString(parent_id));
  bool status = false;

  DeviceElementNodeInfo element_node_info = DeviceElementNodeInfo(
      static_pointer_cast<NamedElement>(device_element_group));

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

    UA_Server_addObjectNode(server_->getServer(), group_node_id, parent_id,
                            reference_type_id, group_browse_name,
                            type_definition, node_attr, NULL, NULL);
    vector<shared_ptr<DeviceElement>> elements =
        device_element_group->getSubelements();

    logger_->log(SeverityLevel::INFO,
                 "Group element {}:{} contains {} subelements.",
                 device_element_group->getElementName(),
                 nodeIdToString(group_node_id), elements.size());
    for (auto element : elements) {
      status = addDeviceNodeElement(element, group_node_id);
    }
  } else {
    logger_->log(
        SeverityLevel::WARNNING, "Parent's {} group element {} is empty!",
        nodeIdToString(parent_id), device_element_group->getElementName());
  }
  return status;
}

bool NodeBuilder::addFunctionNode(shared_ptr<DeviceElement> function,
                                  UA_NodeId parent_id) {
  bool status = false;
  logger_->log(SeverityLevel::WARNNING, "Method element is not implemented!",
               nodeIdToString(parent_id), function->getElementName());
  //@TODO: Implement addFunctionNode stub
  return status;
}

bool NodeBuilder::addReadableNode(shared_ptr<Metric> metric,
                                  UA_NodeId parent_id) {
  bool status = false;

  DeviceElementNodeInfo element_node_info =
      DeviceElementNodeInfo(static_pointer_cast<NamedElement>(metric));

  UA_NodeId metrid_node_id = UA_NODEID_STRING(server_->getServerNamespace(),
                                              element_node_info.getNodeId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), element_node_info.getNodeName());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  node_attr.dataType = getOpcDataType(metric->getDataType());
  node_attr.accessLevel = UA_ACCESSLEVELMASK_READ;

  node_attr.description =
      UA_LOCALIZEDTEXT("EN_US", element_node_info.getNodeDescription());
  node_attr.displayName =
      UA_LOCALIZEDTEXT("EN_US", element_node_info.getNodeName());

  UA_Server_addVariableNode(server_->getServer(), metrid_node_id, parent_id,
                            reference_type_id, metric_browse_name,
                            type_definition, node_attr, NULL, NULL);
  return status;
}

bool NodeBuilder::addWritableNode(shared_ptr<WritableMetric> metric,
                                  UA_NodeId parent_id) {
  bool status = false;

  DeviceElementNodeInfo element_node_info =
      DeviceElementNodeInfo(static_pointer_cast<NamedElement>(metric));

  UA_NodeId metrid_node_id = UA_NODEID_STRING(server_->getServerNamespace(),
                                              element_node_info.getNodeId());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), element_node_info.getNodeName());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  node_attr.dataType = getOpcDataType(metric->getDataType());
  node_attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

  node_attr.description =
      UA_LOCALIZEDTEXT("EN_US", element_node_info.getNodeDescription());
  node_attr.displayName =
      UA_LOCALIZEDTEXT("EN_US", element_node_info.getNodeName());

  UA_Server_addVariableNode(server_->getServer(), metrid_node_id, parent_id,
                            reference_type_id, metric_browse_name,
                            type_definition, node_attr, NULL, NULL);
  return status;
}

UA_NodeId NodeBuilder::getOpcDataType(DataType type) {
  UA_NodeId typeId;
  switch (type) {
  case BOOLEAN: {
    typeId = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    break;
  }
  case BYTE: {
    typeId = UA_TYPES[UA_TYPES_BYTE].typeId;
    break;
  }
  case SHORT: {
    typeId = UA_TYPES[UA_TYPES_INT16].typeId;
    break;
  }
  case INTEGER: {
    typeId = UA_TYPES[UA_TYPES_INT32].typeId;
    break;
  }
  case LONG: {
    typeId = UA_TYPES[UA_TYPES_INT64].typeId;
    break;
  }
  case FLOAT: {
    typeId = UA_TYPES[UA_TYPES_FLOAT].typeId;
    break;
  }
  case DOUBLE: {
    typeId = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    break;
  }
  case STRING: {
    typeId = UA_TYPES[UA_TYPES_STRING].typeId;
    break;
  }
  case UNKNOWN:
  default: {
    //@TODO: Log unknown data type declarations
    break;
  }
  }
  return typeId;
}