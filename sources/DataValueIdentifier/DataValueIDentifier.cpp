#include "CStringFormater.hpp"
#include "DataValueIdentifier.hpp"

#include <stdlib.h>

DataValue identifyDataValue(DataType data_type, const char *data_value_string) {
  DataValue data_value;
  DataValue identifyDataValue(DataType data_type,
                              const char *data_value_string);
  switch (data_type) {
  case DataType::UNSIGNED_SHORT: {
    data_value.unsigned_short_value = (uint8_t)atoi(data_value_string);
    break;
  }
  case DataType::UNSIGNED_INTEGER: {
    data_value.unsigned_integer_value = (uint32_t)atoi(data_value_string);
    break;
  }
  case DataType::UNSIGNED_LONG: {
    data_value.unsigned_long_value = (uint64_t)atoi(data_value_string);
    break;
  }
  case DataType::SIGNED_SHORT: {
    data_value.signed_short_value = (uint8_t)atoi(data_value_string);
    break;
  }
  case DataType::SIGNED_INTEGER: {
    data_value.signed_integer_value = (int32_t)atoi(data_value_string);
    break;
  }
  case DataType::SIGNED_LONG: {
    data_value.signed_long_value = (int64_t)atoi(data_value_string);
    break;
  }
  case DataType::DOUBLE: {
    data_value.double_value = atof(data_value_string);
    break;
  }
  case DataType::BOOLEAN: {
    data_value.boolean_value = identifyBoolean(data_value_string);
    break;
  }
  case DataType::STRING: {
    copyCharArray(data_value_string, data_value.string_value);
    break;
  }
  case DataType::UNKNOWN: {
    break;
  }
  }
}