#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "NodeInformation.h"
#include <open62541/server.h>

typedef enum ReturnStatus {
  OPEN62541_SUCCESS,
  OPEN62541_FAILURE
} OPEN62541_ReturnStatus;

bool startServer();
bool stopServer();
OPEN62541_ReturnStatus addDeviceNode(NodeDescription *device);
OPEN62541_ReturnStatus addDeviceNodeElement(NodeDescription *element,
                                            UA_NodeId parent_id);
OPEN62541_ReturnStatus addGroupNode(NodeDescription *group,
                                    UA_NodeId parent_id);
OPEN62541_ReturnStatus addFunctionNode(NodeDescription *function,
                                       UA_NodeId parent_id);
OPEN62541_ReturnStatus addMetricNode(NodeDescription *metric,
                                     UA_NodeId parent_id);
UA_NodeId getOpcDataType(DataType type);

void update_DataConsumer(NodeDescription *device);

UA_Server *opcua_server;
static UA_UInt16 server_namespace_index = 1;

#endif //_OPEN62541_SERVER_H_