#ifndef __OPEN62541_NODE_ID_HPP
#define __OPEN62541_NODE_ID_HPP

#include <open62541/types.h>

namespace open62541 {

/**
 * @brief A C++-style wrapper around `UA_NodeId`
 */
class NodeId {
public:
  NodeId() = delete;
  explicit NodeId(const NodeId&);
  explicit NodeId(const UA_NodeId&);

  ~NodeId();

  const UA_NodeId& base() const;

  bool operator==(const NodeId& other) const;

private:
  UA_NodeId ua_node_id_;
};

} // namespace open62541

#endif // __OPEN62541_NODE_ID_HPP
