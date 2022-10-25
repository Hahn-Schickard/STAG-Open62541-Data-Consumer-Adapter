#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

extern std::shared_ptr<open62541::Open62541Server> shared_server;
extern std::shared_ptr<open62541::NodeBuilder> shared_node_builder;