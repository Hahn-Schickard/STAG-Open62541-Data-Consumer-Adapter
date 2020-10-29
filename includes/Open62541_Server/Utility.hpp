#ifndef __OPEN62541_UTILITY_FUNCTIONS_HPP
#define __OPEN62541_UTILITY_FUNCTIONS_HPP

#include "DataVariant.hpp"

#include <open62541/nodeids.h>
#include <open62541/types.h>
#include <string>

#ifdef __GNUC__
#define UNUSED(x) x##_UNUSED __attribute__((__unused__))
#else
#define UNUSED(x) x##_UNUSED
#endif

namespace open62541 {

UA_NodeId toNodeId(Information_Model::DataType type);
std::string toString(Information_Model::DataType type);
std::string toString(UA_String input);
std::string toString(UA_Guid input);
std::string toString(const UA_NodeId *nodeId);

} // namespace open62541

#endif //__OPEN62541_UTILITY_FUNCTIONS_HPP