#ifndef __OPEN62541_UTILITY_FUNCTIONS_HPP
#define __OPEN62541_UTILITY_FUNCTIONS_HPP

#include "Information_Model/DataVariant.hpp"

#include <open62541/nodeids.h>
#include <open62541/server.h>
#include <open62541/types.h>
#include <string>

namespace open62541 {

UA_NodeId toNodeId(Information_Model::DataType type);
std::string toString(const UA_String* input);
std::string toString(const UA_NodeId* node_id);
std::string toString(const UA_QualifiedName* name);
UA_String makeUAString(const std::string& input);
UA_ByteString makeUAByteString(const std::vector<uint8_t>& input);

struct StatusCodeNotGood : public std::runtime_error {
  StatusCodeNotGood(const std::string& msg, const UA_StatusCode& code);

  const UA_StatusCode status;
};

void checkStatusCode(const std::string& msg, const UA_StatusCode& status,
    bool uncertain_is_bad = false);
void checkStatusCode(
    const UA_StatusCode& status, bool uncertain_is_bad = false);

/** The UA_TYPES constant that corresponds to intmax_t */
constexpr const size_t UA_TYPES_intmax =
    std::numeric_limits<intmax_t>::digits == 63   ? UA_TYPES_INT64
    : std::numeric_limits<intmax_t>::digits == 31 ? UA_TYPES_INT32
                                                  : UA_TYPES_INT16;
static_assert((std::numeric_limits<intmax_t>::digits == 15) ||
        (UA_TYPES_intmax != UA_TYPES_INT16),
    "intmax_t has unsupported size");

/** The UA_TYPES constant that corresponds to uintmax_t */
constexpr const size_t UA_TYPES_uintmax =
    std::numeric_limits<uintmax_t>::digits == 64   ? UA_TYPES_UINT64
    : std::numeric_limits<uintmax_t>::digits == 32 ? UA_TYPES_UINT32
                                                   : UA_TYPES_UINT16;
static_assert((std::numeric_limits<uintmax_t>::digits == 16) ||
        (UA_TYPES_uintmax != UA_TYPES_UINT16),
    "uintmax_t has unsupported size");

} // namespace open62541

#endif //__OPEN62541_UTILITY_FUNCTIONS_HPP