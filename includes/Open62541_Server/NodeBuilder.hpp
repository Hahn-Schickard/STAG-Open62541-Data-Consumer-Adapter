#ifndef __OPEN62541_NODE_BUILDER_HPP
#define __OPEN62541_NODE_BUILDER_HPP

#include "Information_Model/Device.hpp"
#include "Information_Model/Metric.hpp"
#include "Information_Model/WritableMetric.hpp"
#include "Logger.hpp"
#include "NodeCallbackHandler.hpp"
#include "Open62541Server.hpp"

#include <memory>

namespace open62541 {
class NodeBuilder {
  std::shared_ptr<HaSLL::Logger> logger_;
  std::shared_ptr<Open62541Server> server_;

  UA_StatusCode addDeviceNodeElement(
      std::shared_ptr<Information_Model::DeviceElement> device_element,
      const UA_NodeId *parent_id);

  UA_StatusCode
  addGroupNode(std::shared_ptr<Information_Model::DeviceElementGroup>
                   device_element_group,
               const UA_NodeId *parent_id);

  UA_StatusCode
  addFunctionNode(std::shared_ptr<Information_Model::DeviceElement> function,
                  const UA_NodeId *parent_id);

  UA_StatusCode
  addReadableNode(std::shared_ptr<Information_Model::Metric> metric,
                  const UA_NodeId *parent_id);

  UA_StatusCode
  addWritableNode(std::shared_ptr<Information_Model::WritableMetric> metric,
                  const UA_NodeId *parent_id);

public:
  NodeBuilder(std::shared_ptr<Open62541Server> server);
  ~NodeBuilder();

  UA_StatusCode
  addDeviceNode(std::shared_ptr<Information_Model::Device> device);
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP