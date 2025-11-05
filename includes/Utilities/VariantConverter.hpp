#ifndef __OPEN62541_UTILITY_VARIANT_CONVERTER_HPP
#define __OPEN62541_UTILITY_VARIANT_CONVERTER_HPP

#include <Information_Model/DataVariant.hpp>
#include <open62541/types.h>

namespace open62541 {
UA_NodeId toNodeId(Information_Model::DataType type);

/// The caller is responsible for calling `UA_Variant_clear` on the result
UA_Variant toUAVariant(const Information_Model::DataVariant& variant);

Information_Model::DataVariant toDataVariant(const UA_Variant& variant);
} // namespace open62541

#endif //__OPEN62541_UTILITY_VARIANT_CONVERTER_HPP