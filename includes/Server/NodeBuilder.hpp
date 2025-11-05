#ifndef __OPEN62541_NODE_BUILDER_HPP
#define __OPEN62541_NODE_BUILDER_HPP

#include "NodeCallbackHandler.hpp"
#include "Open62541Server.hpp"

#include <HaSLL/Logger.hpp>
#include <Information_Model/Device.hpp>
#include <Information_Model/Readable.hpp>
#include <Information_Model/Writable.hpp>

#include <memory>

namespace open62541 {
class NodeBuilder {
  HaSLL::LoggerPtr logger_;
  std::shared_ptr<Open62541Server> server_;

  /// The caller is responsible for calling `UA_NodeId_clear` on the `UA_NodeId`
  std::pair<UA_StatusCode, UA_NodeId> addObjectNode(
      const Information_Model::MetaInfoPtr& element,
      std::optional<UA_NodeId> parent_node_id = std::nullopt);

  UA_StatusCode addElementNode(
      const Information_Model::ElementPtr& device_element,
      const UA_NodeId& parent_id);

  UA_StatusCode addGroupNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::GroupPtr& device_element_group,
      const UA_NodeId& parent_id);

  UA_StatusCode addReadableNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::ReadablePtr& metric, const UA_NodeId& parent_id);

  UA_StatusCode addObservableNode(
      const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::ObservablePtr& metric,
      const UA_NodeId& parent_id);

  UA_StatusCode addWritableNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::WritablePtr& metric, const UA_NodeId& parent_id);

  UA_StatusCode addCallableNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::CallablePtr& callable,
      const UA_NodeId& parent_id);

  /// The caller is responsible for calling `UA_VariableAttributes_clear`
  /// on `value_attribute`
  template <class MetricType> // either Metric or WritableMetric
  UA_StatusCode setValue(UA_VariableAttributes& value_attribute,
      const Information_Model::MetaInfoPtr& meta_info,
      const std::shared_ptr<MetricType>& metric,
      const std::string& metric_type_description);

  UA_StatusCode removeDataSources(const UA_NodeId* node_id);

public:
  NodeBuilder(const std::shared_ptr<Open62541Server>& server);
  ~NodeBuilder();

  UA_StatusCode addDeviceNode(const Information_Model::DevicePtr& device);
  UA_StatusCode deleteDeviceNode(const std::string& device_id);
  void cleanup();
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP