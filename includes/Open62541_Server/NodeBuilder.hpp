#ifndef __OPEN62541_NODE_BUILDER_HPP
#define __OPEN62541_NODE_BUILDER_HPP

#include "Device.hpp"
#include "Logger.hpp"
#include "Metric.hpp"
#include "Open62541Server.hpp"
#include "WritableMetric.hpp"

namespace open62541 {
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
  bool addReadableNode(std::shared_ptr<Information_Model::Metric> metric,
                       UA_NodeId parent_id);
  bool
  addWritableNode(std::shared_ptr<Information_Model::WritableMetric> metric,
                  UA_NodeId parent_id);

  UA_NodeId getOpcDataType(Information_Model::DataType type);

private:
  Open62541Server *server_;
  std::shared_ptr<HaSLL::Logger> logger_;
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP