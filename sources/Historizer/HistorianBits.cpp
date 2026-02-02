#include "HistorianBits.hpp"

namespace HistorianBits {
using namespace std;

void setHistorianBits(UA_StatusCode* status, DataLocation data_loc,
    bool is_partial, bool has_extra, bool has_multiple) {
  if (UA_StatusCode_isBad(*status)) {
    string error_msg =
        "Can not set historian bits for " + string(UA_StatusCode_name(*status));
    throw invalid_argument(error_msg);
  }
  if (data_loc == DataLocation::Reserved) {
    throw invalid_argument(
        "Data location type can not be set to reserved type.");
  }
  *status |= static_cast<uint32_t>(data_loc);

  static constexpr uint8_t PARTIAL_DATA_OFFSET = 2;
  static constexpr uint8_t EXTRA_DATA_OFFSET = 3;
  static constexpr uint8_t MULTI_VALUE_OFFSET = 4;

  *status |= (static_cast<uint32_t>(is_partial) << PARTIAL_DATA_OFFSET) |
      (static_cast<uint32_t>(has_extra) << EXTRA_DATA_OFFSET) |
      (static_cast<uint32_t>(has_multiple) << MULTI_VALUE_OFFSET);
}

UA_StatusCode setHistorianBits(const UA_StatusCode* status,
    DataLocation data_loc, bool is_partial, bool has_extra, bool has_multiple) {
  UA_StatusCode result = *status; // obtain a copy of original status code
  setHistorianBits(&result, data_loc, is_partial, has_extra, has_multiple);
  return result;
}

DataLocation getDataLocation(const UA_StatusCode status) {
  static constexpr uint32_t HISTORIAN_BITS_MASK = 0x03;

  return static_cast<DataLocation>(status & HISTORIAN_BITS_MASK);
}

string toString(DataLocation location) {
  switch (location) {
  case DataLocation::Raw: {
    return "Raw";
  }
  case DataLocation::Calculated: {
    return "Calculated";
  }
  case DataLocation::Interpolated: {
    return "Interpolated";
  }
  default: {
    return "Reserved";
  }
  }
}

UA_Boolean hasPartialValue(const UA_StatusCode status) {
  static constexpr uint32_t PARTIAL_DATA_MASK = 0x1B;

  return (status & PARTIAL_DATA_MASK) > 0;
}

UA_Boolean hasExtraData(const UA_StatusCode status) {
  static constexpr uint32_t EXTRA_DATA_MASK = 0x17;

  return (status & EXTRA_DATA_MASK) > 0;
}

UA_Boolean hasMultipleValues(const UA_StatusCode status) {
  static constexpr uint32_t MULTI_VALUE_MASK = 0x0F;

  return (status & MULTI_VALUE_MASK) > 0;
}
} // namespace HistorianBits
