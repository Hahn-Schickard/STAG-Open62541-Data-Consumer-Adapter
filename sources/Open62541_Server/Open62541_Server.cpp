#include "Open62541_Server.hpp"
#include "NullLogger.hpp"

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <memory>
#include <pthread.h>
#include <signal.h>
#include <vector>

using namespace std;
using namespace Information_Model;

static volatile UA_Boolean SERVER_RUNNING_FLAG = false;

// nolint
static void stopHandler(int sig) { SERVER_RUNNING_FLAG = false; }

void *serverThread(void *args) {
  UA_Server_run(opcua_server, &SERVER_RUNNING_FLAG);
  return NULL;
}

bool startServer() {
  if (SERVER_RUNNING_FLAG) {
    stopServer();
  }

  signal(SIGINT, stopHandler);
  signal(SIGTERM, stopHandler);
  SERVER_RUNNING_FLAG = true;

  opcua_server = UA_Server_new();
  UA_ServerConfig *config = UA_Server_getConfig(opcua_server);
  UA_ServerConfig_setDefault(config);
  config->logger = Null_Logger_;

  pthread_t server_thread_id;
  int return_code = 0;

  return_code = pthread_create(&server_thread_id, NULL, serverThread, NULL);
  if (return_code) {
    return UA_NS0ID_ALARMCONDITIONTYPE_ACTIVESTATE_FALSESTATE;
  }
  return_code = pthread_detach(server_thread_id);
  if (return_code) {
    return false;
  }
  return true;
}

bool stopServer() {
  SERVER_RUNNING_FLAG = false;
  UA_StatusCode retval = UA_Server_run_shutdown(opcua_server);
  if (retval == UA_STATUSCODE_GOOD) {
    UA_Server_delete(opcua_server);
    return true;
  } else {
    return false;
  }
}

void update_DataConsumer(shared_ptr<Device> device) { addDeviceNode(device); }

OPEN62541_ReturnStatus addDeviceNode(shared_ptr<Device> device) {
  UA_NodeId device_node_id = UA_NODEID_STRING(
      server_namespace_index, (char *)device->getElementRefId().c_str());
  UA_NodeId parent_node_id = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName device_browse_name = UA_QUALIFIEDNAME(
      server_namespace_index, (char *)device->getElementName().c_str());
  UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

  node_attr.description = UA_LOCALIZEDTEXT(
      "EN_US", (char *)device->getElementDescription().c_str());
  node_attr.displayName =
      UA_LOCALIZEDTEXT("EN_US", (char *)device->getElementName().c_str());

  UA_Server_addObjectNode(opcua_server, device_node_id, parent_node_id,
                          reference_type_id, device_browse_name,
                          type_definition, node_attr, NULL, NULL);

  shared_ptr<DeviceElementGroup> device_element_group =
      device->getDeviceElementGroup();

  return addGroupNode(device_element_group, device_node_id);
}

OPEN62541_ReturnStatus addDeviceNodeElement(shared_ptr<DeviceElement> element,
                                            UA_NodeId parent_id) {
  OPEN62541_ReturnStatus status = OPEN62541_FAILURE;
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

OPEN62541_ReturnStatus
addGroupNode(shared_ptr<DeviceElementGroup> device_element_group,
             UA_NodeId parent_id) {
  OPEN62541_ReturnStatus status = OPEN62541_FAILURE;

  UA_NodeId group_node_id =
      UA_NODEID_STRING(server_namespace_index,
                       (char *)device_element_group->getElementRefId().c_str());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName group_browse_name =
      UA_QUALIFIEDNAME(server_namespace_index,
                       (char *)device_element_group->getElementName().c_str());
  UA_NodeId type_definition = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE);
  UA_ObjectAttributes node_attr = UA_ObjectAttributes_default;

  node_attr.description = UA_LOCALIZEDTEXT(
      "EN_US", (char *)device_element_group->getElementDescription().c_str());
  node_attr.displayName = UA_LOCALIZEDTEXT(
      "EN_US", (char *)device_element_group->getElementName().c_str());

  UA_Server_addObjectNode(opcua_server, group_node_id, parent_id,
                          reference_type_id, group_browse_name, type_definition,
                          node_attr, NULL, NULL);
  vector<shared_ptr<DeviceElement>> elements =
      device_element_group->getSubelements();
  for (auto element : elements) {
    status = addDeviceNodeElement(element, group_node_id);
  }
  return status;
}

OPEN62541_ReturnStatus addFunctionNode(shared_ptr<DeviceElement> function,
                                       UA_NodeId parent_id) {
  OPEN62541_ReturnStatus status = OPEN62541_FAILURE;
  //@TODO: Implement addFunctionNode stub
  return status;
}

OPEN62541_ReturnStatus addMetricNode(shared_ptr<DeviceElement> metric,
                                     UA_NodeId parent_id) {
  OPEN62541_ReturnStatus status = OPEN62541_FAILURE;

  UA_NodeId metrid_node_id = UA_NODEID_STRING(
      server_namespace_index, (char *)metric->getElementRefId().c_str());
  UA_NodeId reference_type_id = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
  UA_QualifiedName metric_browse_name = UA_QUALIFIEDNAME(
      server_namespace_index, (char *)metric->getElementName().c_str());
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

  UA_Server_addVariableNode(opcua_server, metrid_node_id, parent_id,
                            reference_type_id, metric_browse_name,
                            type_definition, node_attr, NULL, NULL);
  return status;
}

UA_NodeId getOpcDataType(DataType type) {
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