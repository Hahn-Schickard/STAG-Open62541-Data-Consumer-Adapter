#ifndef __OPEN62541_UTILITY_VARIANT_CONVERTER_HPP
#define __OPEN62541_UTILITY_VARIANT_CONVERTER_HPP

#include <Information_Model/DataVariant.hpp>
#include <open62541/types.h>

namespace open62541 {
/**
 * @brief Convert a given Information Model data type into a UA Type Reference
 * node
 *
 * @attention Resulting UA_NodeId content must be cleared by the caller when the
 * content is no longer needed
 *
 * @param type
 * @return UA_NodeId
 */
UA_NodeId toNodeId(Information_Model::DataType type);

/**
 * @brief Convert a given Information Model variant into a UA_Variant
 *
 * @attention Resulting UA_Variant must be cleared by the caller, when content
 * is no longer needed
 *
 * @param variant
 * @return UA_Variant
 */
UA_Variant toUAVariant(const Information_Model::DataVariant& variant);

/**
 * @brief Convert a given UA_Variant into an Information Model variant
 *
 * @attention Given UA_Variant content is not cleared
 *
 * @param variant
 * @return Information_Model::DataVariant
 */
Information_Model::DataVariant toDataVariant(const UA_Variant& variant);
} // namespace open62541

#endif //__OPEN62541_UTILITY_VARIANT_CONVERTER_HPP