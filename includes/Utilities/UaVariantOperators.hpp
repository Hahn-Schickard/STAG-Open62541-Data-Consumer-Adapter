#ifndef __OPEN62541_UTILITY_UA_VARIANT_OPERATORS_HPP
#define __OPEN62541_UTILITY_UA_VARIANT_OPERATORS_HPP

#include <open62541/types.h>

namespace open62541 {

UA_Variant operator*(const UA_Variant& lhs, const intmax_t& rhs);

UA_Variant operator/(const UA_Variant& lhs, const intmax_t& rhs);

UA_Variant operator+(const UA_Variant& lhs, const UA_Variant& rhs);

UA_Variant operator-(const UA_Variant& lhs, const UA_Variant& rhs);
} // namespace open62541

#endif //__OPEN62541_UTILITY_UA_VARIANT_OPERATORS_HPP