#ifndef __OPEN62541_UTILITY_FUNCTIONS_HPP
#define __OPEN62541_UTILITY_FUNCTIONS_HPP

#include "DataVariant.hpp"

#include <open62541/nodeids.h>
#include <open62541/types.h>
#include <string>

namespace open62541 {

std::string toString(Information_Model::DataType type);
std::string toString(UA_String input);
std::string toString(UA_Guid input);
std::string toString(UA_NodeId nodeId);

} // namespace open62541

#endif //__OPEN62541_UTILITY_FUNCTIONS_HPP