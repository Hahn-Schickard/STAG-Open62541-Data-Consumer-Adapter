#ifndef __OPEN62541_NODE_ID_HPP
#define __OPEN62541_NODE_ID_HPP

#include <open62541/types.h>

namespace open62541 {

/**
 * @brief A C++-style wrapper around `UA_NodeId`
 */
class NodeId {
private:
  UA_NodeId ua_node_id_;

public:
  NodeId() = delete;
  NodeId(NodeId const&);
  NodeId(UA_NodeId const&);
  ~NodeId();

  const UA_NodeId& base() const;

  bool operator==(NodeId const& other) const;
};

} // namespace open62541

#endif // __OPEN62541_NODE_ID_HPP
