#include "NodeId.hpp"

#include <stdexcept>

#include <open62541/server.h>

namespace open62541 {

NodeId::NodeId(UA_NodeId const& source) {
  if (UA_NodeId_copy(&source, &ua_node_id_) != UA_STATUSCODE_GOOD) {
    throw std::bad_alloc();
  }
}

NodeId::NodeId(NodeId const& source) : NodeId(source.ua_node_id_) {}

NodeId::~NodeId() { UA_NodeId_clear(&ua_node_id_); }

const UA_NodeId& NodeId::base() const { return ua_node_id_; }

bool NodeId::operator==(NodeId const& other) const {
  return UA_NodeId_equal(&ua_node_id_, &other.ua_node_id_);
}

} // namespace open62541
