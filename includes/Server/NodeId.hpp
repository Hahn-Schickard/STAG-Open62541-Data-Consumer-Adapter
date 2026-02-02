#ifndef __OPEN62541_NODE_ID_HPP
#define __OPEN62541_NODE_ID_HPP

#include <open62541/types.h>

namespace open62541 {

/**
 * @brief A C++ style wrapper around `UA_NodeId`
 */
class NodeId {
public:
  NodeId() = delete;

  NodeId(const NodeId& other);

  NodeId(const UA_NodeId& node_id);

  NodeId(NodeId&& other) noexcept;

  ~NodeId();

  const UA_NodeId& base() const;

  NodeId& operator=(const NodeId& other);

  NodeId& operator=(NodeId&& other) noexcept;

  bool operator==(const NodeId& other) const;

private:
  UA_NodeId id_;
};

} // namespace open62541

#endif // __OPEN62541_NODE_ID_HPP
