#include "NodeBuilder.hpp"

#include <memory>
#include <open62541/nodeids.h>
#include <vector>

using namespace std;
using namespace Information_Model;

NodeBuilder::NodeBuilder(Open62541Server *server) : server_(server) {}

NodeBuilder::~NodeBuilder() {}

bool NodeBuilder::addDeviceNode(shared_ptr<Device> device) {
  UA_NodeId device_node_id = UA_NODEID_STRING(
      server_->getServerNamespace(), (char *)device->getElementRefId().c_str());
  UA_NodeId parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName device_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), (char *)device->getElementName().c_str());
  UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

  node_attr.description = UA_LOCALIZEDTEXT(
      "EN_US", (char *)device->getElementDescription().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT("EN_US", (char *)device->getElementName().c_str());

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
  bool status = false;
  switch (element->getElementType()) {
  case Group: {
    status = addGroupNode(static_pointer_cast<DeviceElementGroup>(element),
                          parent_id);
    break;
  }
  case Function: {
    status = addFunctionNode(element, parent_id);
    break;
  }
  case Observable:
  case Writable:
  case Readonly: {
    status = addMetricNode(element, parent_id);
    break;
  }
  case Undefined:
  default: {
    //@TODO: Handle default behaviour for UNRECOGNIZED_NODE
    break;
  }
  }
  return status;
}

bool NodeBuilder::addGroupNode(
    shared_ptr<DeviceElementGroup> device_element_group, UA_NodeId parent_id) {
  bool status = false;

  if (!device_element_group->getSubelements().empty()) {
    UA_NodeId group_node_id = UA_NODEID_STRING(
        server_->getServerNamespace(),
        (char *)device_element_group->getElementRefId().c_str());
    UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_QualifiedName group_browse_name = UA_QUALIFIEDNAME(
        server_->getServerNamespace(),
        (char *)device_element_group->getElementName().c_str());
    UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
    UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

    node_attr.description = UA_LOCALIZEDTEXT(
        "EN_US", (char *)device_element_group->getElementDescription().c_str());
    node_attr.displayName = UA_LOCALIZEDTEXT(
        "EN_US", (char *)device_element_group->getElementName().c_str());

    UA_Server_addObjectNode(server_->getServer(), group_node_id, parent_id,
                            reference_type_id, group_browse_name,
                            type_definition, node_attr, NULL, NULL);
    vector<shared_ptr<DeviceElement>> elements =
        device_element_group->getSubelements();
    for (auto element : elements) {
      status = addDeviceNodeElement(element, group_node_id);
    }
  } // LOG that the group was empty!
  return status;
}

bool NodeBuilder::addFunctionNode(shared_ptr<DeviceElement> function,
                                  UA_NodeId parent_id) {
  bool status = false;
  //@TODO: Implement addFunctionNode stub
  return status;
}

bool NodeBuilder::addMetricNode(shared_ptr<DeviceElement> metric,
                                UA_NodeId parent_id) {
  bool status = false;

  UA_NodeId metrid_node_id = UA_NODEID_STRING(
      server_->getServerNamespace(), (char *)metric->getElementRefId().c_str());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME(
      server_->getServerNamespace(), (char *)metric->getElementName().c_str());
  UA_NodeId type_definition =
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
  UA_VariableAttributes node_attr = UA_VariableAttributes_default;

  // TODO: handle data types
  node_attr.dataType = getOpcDataType(DataType::STRING);
  node_attr.accessLevel = UA_ACCESSLEVELMASK_READ |
                          UA_ACCESSLEVELMASK_WRITE; //@TODO: change to a
                                                    // function that assigns
                                                    // proper accessLevel
  node_attr.description = UA_LOCALIZEDTEXT(
      "EN_US", (char *)metric->getElementDescription().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT("EN_US", (char *)metric->getElementName().c_str());

  UA_Server_addVariableNode(server_->getServer(), metrid_node_id, parent_id,
                            reference_type_id, metric_browse_name,
                            type_definition, node_attr, NULL, NULL);
  return status;
}

UA_NodeId NodeBuilder::getOpcDataType(DataType type) {
  //@TODO: set some initial value that would be common for all nodes, string
  // maybe?
  UA_NodeId typeId;
  switch (type) {
  case UNSIGNED_SHORT: {
    typeId = UA_TYPES[UA_TYPES_UINT16].typeId;
    break;
  }
  case UNSIGNED_INTEGER: {
    typeId = UA_TYPES[UA_TYPES_UINT32].typeId;
    break;
  }
  case UNSIGNED_LONG: {
    typeId = UA_TYPES[UA_TYPES_UINT64].typeId;
    break;
  }
  case SIGNED_SHORT: {
    typeId = UA_TYPES[UA_TYPES_INT16].typeId;
    break;
  }
  case SIGNED_INTEGER: {
    typeId = UA_TYPES[UA_TYPES_INT32].typeId;
    break;
  }
  case SIGNED_LONG: {
    typeId = UA_TYPES[UA_TYPES_INT64].typeId;
    break;
  }
  case DOUBLE: {
    typeId = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    break;
  }
  case BOOLEAN: {
    typeId = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
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