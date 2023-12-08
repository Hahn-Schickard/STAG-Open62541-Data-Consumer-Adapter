#include "NodeId.hpp"

#include <stdexcept>

#include <open62541/server.h>

namespace open62541 {

NodeId::NodeId(UA_NodeId const& source) {
  if (UA_NodeId_copy(&source, &ua_) != UA_STATUSCODE_GOOD) {
    throw std::bad_alloc();
  }
}

NodeId::NodeId(NodeId const& source) : NodeId(source.ua_) {}

NodeId::~NodeId() { UA_NodeId_clear(&ua_); }

const UA_NodeId& NodeId::base() const { return ua_; }
UA_NodeId& NodeId::base() { return ua_; }

bool NodeId::operator==(NodeId const& other) const {
  return UA_NodeId_equal(&ua_, &other.ua_);
}

} // namespace open62541
