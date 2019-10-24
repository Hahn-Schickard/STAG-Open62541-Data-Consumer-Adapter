#ifndef __OPEN62541_NODE_BUILDER_HPP
#define __OPEN62541_NODE_BUILDER_HPP

#include "Device.hpp"
#include "Open62541Server.hpp"

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

class NodeBuilder {
public:
  NodeBuilder(Open62541Server *server);

  ~NodeBuilder();

  bool addDeviceNode(std::shared_ptr<Information_Model::Device> device);

  bool addDeviceNodeElement(
      std::shared_ptr<Information_Model::DeviceElement> device_element,
      UA_NodeId parent_id);
  bool addGroupNode(std::shared_ptr<Information_Model::DeviceElementGroup>
                        device_element_group,
                    UA_NodeId parent_id);
  bool
  addFunctionNode(std::shared_ptr<Information_Model::DeviceElement> function,
                  UA_NodeId parent_id);
  bool addMetricNode(std::shared_ptr<Information_Model::DeviceElement> metric,
                     UA_NodeId parent_id);
  UA_NodeId getOpcDataType(DataType type);

private:
  Open62541Server *server_;
};

#endif //__OPEN62541_NODE_BUILDER_HPP