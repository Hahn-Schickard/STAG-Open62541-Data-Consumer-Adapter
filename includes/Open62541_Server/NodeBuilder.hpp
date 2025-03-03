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
  HaSLL::LoggerPtr logger_;
  std::shared_ptr<Open62541Server> server_;

  /// The caller is responsible for calling `UA_NodeId_clear` on the `UA_NodeId`
  std::pair<UA_StatusCode, UA_NodeId> addObjectNode(
      const Information_Model::NonemptyNamedElementPtr& element,
      std::optional<UA_NodeId> parent_node_id = std::nullopt);

  UA_StatusCode addDeviceNodeElement(
      const Information_Model::NonemptyDeviceElementPtr& device_element,
      const UA_NodeId& parent_id);

  UA_StatusCode addGroupNode(
      const Information_Model::NonemptyNamedElementPtr& meta_info,
      const Information_Model::NonemptyDeviceElementGroupPtr&
          device_element_group,
      const UA_NodeId& parent_id);

  UA_StatusCode addReadableNode(
      const Information_Model::NonemptyNamedElementPtr& meta_info,
      const Information_Model::NonemptyMetricPtr& metric,
      const UA_NodeId& parent_id);

  UA_StatusCode addWritableNode(
      const Information_Model::NonemptyNamedElementPtr& meta_info,
      const Information_Model::NonemptyWritableMetricPtr& metric,
      const UA_NodeId& parent_id);

  UA_StatusCode addFunctionNode(
      const Information_Model::NonemptyNamedElementPtr& meta_info,
      const Information_Model::NonemptyFunctionPtr& function,
      const UA_NodeId& parent_id);

  /// The caller is responsible for calling `UA_VariableAttributes_clear`
  /// on `value_attribute`
  template <class MetricType> // either Metric or WritableMetric
  UA_StatusCode setValue(UA_VariableAttributes& value_attribute,
      const Information_Model::NonemptyNamedElementPtr& meta_info,
      const Nonempty::Pointer<std::shared_ptr<MetricType>>& metric,
      const std::string& metric_type_description);

  UA_StatusCode removeDataSources(const UA_NodeId* node_id);

public:
  NodeBuilder(const std::shared_ptr<Open62541Server>& server);
  ~NodeBuilder();

  UA_StatusCode addDeviceNode(
      const Information_Model::NonemptyDevicePtr& device);
  UA_StatusCode deleteDeviceNode(const std::string& device_id);
  void cleanup();
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP