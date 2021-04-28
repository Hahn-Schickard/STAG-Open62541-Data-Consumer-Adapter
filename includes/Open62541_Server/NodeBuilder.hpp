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

  UA_StatusCode
  addDeviceNodeElement(Information_Model::DeviceElementPtr device_element,
                       const UA_NodeId *parent_id);

  UA_StatusCode
  addGroupNode(Information_Model::DeviceElementGroupPtr device_element_group,
               const UA_NodeId *parent_id);

  UA_StatusCode addFunctionNode(Information_Model::DeviceElementPtr function,
                                const UA_NodeId *parent_id);

  UA_StatusCode addReadableNode(Information_Model::MetricPtr metric,
                                const UA_NodeId *parent_id);

  UA_StatusCode addWritableNode(Information_Model::WritableMetricPtr metric,
                                const UA_NodeId *parent_id);

  UA_StatusCode setValue(UA_VariableAttributes &value_attribute,
                         Information_Model::MetricPtr metric);

  UA_StatusCode setValue(UA_VariableAttributes &value_attribute,
                         Information_Model::WritableMetricPtr metric);

public:
  NodeBuilder(std::shared_ptr<Open62541Server> server);
  ~NodeBuilder();

  UA_StatusCode addDeviceNode(Information_Model::DevicePtr device);
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP