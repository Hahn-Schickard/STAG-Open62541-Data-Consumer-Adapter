#ifndef __OPEN62541_NODE_BUILDER_HPP
#define __OPEN62541_NODE_BUILDER_HPP

#include "HaSLL/Logger.hpp"
#include "Information_Model/Device.hpp"
#include "Information_Model/Metric.hpp"
#include "Information_Model/WritableMetric.hpp"
#include "NodeCallbackHandler.hpp"
#include "Open62541Server.hpp"

#include <memory>

namespace open62541 {
class NodeBuilder {
  HaSLI::LoggerPtr logger_;
  std::shared_ptr<Open62541Server> server_;

  std::pair<UA_StatusCode, UA_NodeId> addObjectNode(
      Information_Model::NamedElementPtr element,
      std::optional<UA_NodeId> parent_node_id = std::nullopt);

  UA_StatusCode addDeviceNodeElement(
      Information_Model::DeviceElementPtr device_element, UA_NodeId parent_id);

  UA_StatusCode addGroupNode(
      Information_Model::DeviceElementGroupPtr device_element_group,
      UA_NodeId parent_id);

  UA_StatusCode addFunctionNode(
      Information_Model::DeviceElementPtr function, UA_NodeId parent_id);

  UA_StatusCode addReadableNode(
      Information_Model::MetricPtr metric, UA_NodeId parent_id);

  UA_StatusCode addWritableNode(
      Information_Model::WritableMetricPtr metric, UA_NodeId parent_id);

  template <class MetricType> // either Metric or WritableMetric
  UA_StatusCode setValue(UA_VariableAttributes& value_attribute,
      std::shared_ptr<MetricType> metric, std::string metric_type_description);

public:
  NodeBuilder(std::shared_ptr<Open62541Server> server);
  ~NodeBuilder();

  UA_StatusCode addDeviceNode(Information_Model::DevicePtr device);
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP