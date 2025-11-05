#ifndef __OPEN62541_HISTORIAN_BITS_HPP
#define __OPEN62541_HISTORIAN_BITS_HPP

#include <cstdint>
#include <open62541/types.h>

#include <stdexcept>
#include <string>

/**
 * @brief Defines Aggregate Result Status Code expansion
 *
 * Defined in UA Part 4: Services Table 181 DataValue InfoBits
 *
 */
namespace HistorianBits {
enum class DataLocation : uint8_t {
  Raw = 0x00,
  Calculated = 0x01,
  Interpolated = 0x02,
  Reserved = 0x03
};

void setHistorianBits(UA_StatusCode* status, DataLocation data_loc,
    bool is_partial = false, bool has_extra = false, bool has_multiple = false);

UA_StatusCode setHistorianBits(const UA_StatusCode* status,
    DataLocation data_loc, bool is_partial = false, bool has_extra = false,
    bool has_multiple = false);

DataLocation getDataLocation(const UA_StatusCode status);

std::string toString(DataLocation location);

/**
 * @brief Checks if StatusCode indicates that data value is calculated with
 * an incomplete interval
 *
 * @param status
 * @return UA_Boolean
 */
UA_Boolean hasPartialValue(const UA_StatusCode status);

/**
 * @brief Checks if StatusCode indicates that the Raw data value supersedes
 * other data at the same timestamp
 *
 * @param status
 * @return UA_Boolean
 */
UA_Boolean hasExtraData(const UA_StatusCode status);
/**
 * @brief Checks if StatusCode indicates that the multiple data values match
 * the Aggregate criteria
 *
 * @param status
 * @return UA_Boolean
 */
UA_Boolean hasMultipleValues(const UA_StatusCode status);
} // namespace HistorianBits

#endif //__OPEN62541_HISTORIAN_BITS_HPP