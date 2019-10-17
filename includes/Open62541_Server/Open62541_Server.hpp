#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Device.hpp"
#include <memory>
#include <open62541/server.h>

typedef enum ReturnStatus {
  OPEN62541_SUCCESS,
  OPEN62541_FAILURE
} OPEN62541_ReturnStatus;

typedef enum DataTypeEnum {
  UNSIGNED_SHORT,
  UNSIGNED_INTEGER,
  UNSIGNED_LONG,
  SIGNED_SHORT,
  SIGNED_INTEGER,
  SIGNED_LONG,
  DOUBLE,
  BOOLEAN,
  STRING,
  UNKNOWN
} DataType;

bool startServer();
bool stopServer();
OPEN62541_ReturnStatus
addDeviceNode(std::shared_ptr<Information_Model::Device> device);

OPEN62541_ReturnStatus addDeviceNodeElement(
    std::shared_ptr<Information_Model::DeviceElement> device_element,
    UA_NodeId parent_id);
OPEN62541_ReturnStatus addGroupNode(
    std::shared_ptr<Information_Model::DeviceElementGroup> device_element_group,
    UA_NodeId parent_id);
OPEN62541_ReturnStatus
addFunctionNode(std::shared_ptr<Information_Model::DeviceElement> function,
                UA_NodeId parent_id);
OPEN62541_ReturnStatus
addMetricNode(std::shared_ptr<Information_Model::DeviceElement> metric,
              UA_NodeId parent_id);
UA_NodeId getOpcDataType(DataType type);

void update_DataConsumer(std::shared_ptr<Information_Model::Device> device);

UA_Server *opcua_server;
static UA_UInt16 server_namespace_index = 1;

#endif //_OPEN62541_SERVER_H_