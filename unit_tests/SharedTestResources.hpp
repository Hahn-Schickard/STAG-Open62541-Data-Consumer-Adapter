#ifndef __OPEN62541_SHARED_TEST_RESORUCES_HPP
#define __OPEN62541_SHARED_TEST_RESORUCES_HPP

#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

using Open62541ServerPtr = std::shared_ptr<open62541::Open62541Server>;
using NodeBuilderPtr = std::shared_ptr<open62541::NodeBuilder>;

inline Open62541ServerPtr shared_server;
inline NodeBuilderPtr shared_node_builder;

#endif //__OPEN62541_SHARED_TEST_RESORUCES_HPP