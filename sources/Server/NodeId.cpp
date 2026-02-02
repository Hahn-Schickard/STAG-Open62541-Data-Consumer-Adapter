#include "NodeId.hpp"
#include "StringConverter.hpp"

#include <open62541/server.h>

#include <stdexcept>

namespace open62541 {

NodeId::NodeId(const UA_NodeId& node_id) { UA_NodeId_copy(&node_id, &id_); }

NodeId::NodeId(const NodeId& other) { UA_NodeId_copy(&(other.id_), &id_); }

NodeId::NodeId(NodeId&& other) noexcept {
  UA_NodeId_copy(&(other.id_), &id_);
  UA_NodeId_clear(&other.id_);
}

NodeId::~NodeId() { UA_NodeId_clear(&id_); }

const UA_NodeId& NodeId::base() const { return id_; }

NodeId& NodeId::operator=(const NodeId& other) {
  if (this != &other) {
    UA_NodeId_copy(&(other.id_), &id_);
  }
  return *this;
}

NodeId& NodeId::operator=(NodeId&& other) noexcept {
  if (this != &other) {
    UA_NodeId_copy(&(other.id_), &id_);
    UA_NodeId_clear(&other.id_);
  }
  return *this;
}

bool NodeId::operator==(const NodeId& other) const {
  return UA_NodeId_equal(&id_, &other.id_);
}

} // namespace open62541
