#ifndef __OPEN62541_NODE_BUILDER_HPP
#define __OPEN62541_NODE_BUILDER_HPP

#include "CallbackRepo.hpp"
#ifdef ENABLE_UA_HISTORIZING
#include "Historizer.hpp"
#endif // ENABLE_UA_HISTORIZING

#include <HaSLL/Logger.hpp>
#include <Information_Model/Device.hpp>
#include <Information_Model/Readable.hpp>
#include <Information_Model/Writable.hpp>
#include <open62541/server.h>

#include <memory>

namespace open62541 {
struct NodeBuilder {
  NodeBuilder(const CallbackRepoPtr& repo,
#ifdef ENABLE_UA_HISTORIZING
      const HistorizerPtr& historizer,
#endif // ENABLE_UA_HISTORIZING
      UA_Server* server);
  ~NodeBuilder() = default;

  UA_StatusCode addDeviceNode(const Information_Model::DevicePtr& device);
  UA_StatusCode deleteDeviceNode(const std::string& device_id);

private:
  UA_NodeId addObjectNode(const Information_Model::MetaInfoPtr& element,
      std::optional<UA_NodeId> parent_node_id = std::nullopt);

  UA_StatusCode addElementNode(
      const Information_Model::ElementPtr& device_element,
      const UA_NodeId& parent_id);

  UA_StatusCode addGroupNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::GroupPtr& device_element_group,
      const UA_NodeId& parent_id);

  UA_StatusCode addReadableNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::ReadablePtr& readable,
      const UA_NodeId& parent_id);

  UA_StatusCode addObservableNode(
      const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::ObservablePtr& observable,
      const UA_NodeId& parent_id);

  UA_StatusCode addWritableNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::WritablePtr& writable,
      const UA_NodeId& parent_id);

  UA_StatusCode addCallableNode(const Information_Model::MetaInfoPtr& meta_info,
      const Information_Model::CallablePtr& callable,
      const UA_NodeId& parent_id);

  UA_StatusCode removeDataSources(const UA_NodeId* node_id);

  HaSLL::LoggerPtr logger_;
  CallbackRepoPtr repo_;
#ifdef ENABLE_UA_HISTORIZING
  HistorizerPtr historizer_;
#endif // ENABLE_UA_HISTORIZING
  UA_Server* server_;
};
} // namespace open62541

#endif //__OPEN62541_NODE_BUILDER_HPP