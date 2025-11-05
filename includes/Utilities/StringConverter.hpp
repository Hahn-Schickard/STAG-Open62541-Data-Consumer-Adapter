#ifndef __OPEN62541_UTILITY_STRING_CONVERSION_HPP
#define __OPEN62541_UTILITY_STRING_CONVERSION_HPP

#include <open62541/types.h>

#include <cstdint>
#include <string>
#include <vector>

namespace open62541 {

std::string toString(const UA_String* input);

std::string toString(const UA_NodeId* node_id);

std::string toString(const UA_QualifiedName* name);

std::string toString(const UA_ExpandedNodeId& id);

std::string toString(UA_DateTime timestamp);

std::string toSqlType(const UA_DataType* variant);

UA_String makeUAString(const std::string& input);

UA_ByteString makeUAByteString(const std::vector<uint8_t>& input);
} // namespace open62541

#endif //__OPEN62541_UTILITY_STRING_CONVERSION_HPP