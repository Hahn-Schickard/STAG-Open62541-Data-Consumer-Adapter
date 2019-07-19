#include "CStringFormater.hpp"
#include "DataValueIdentifier.hpp"

#include <stdlib.h>

DataValueWrapper identifyDataValue(DataType data_type,
                                   const char *data_value_string)
{
  DataValueWrapper data;

  switch (data_type)
  {
  case DataType::UNSIGNED_SHORT:
  {
    data.type = DataType::UNSIGNED_SHORT;
    data.value.unsigned_short_value = (uint8_t)atoi(data_value_string);
    break;
  }
  case DataType::UNSIGNED_INTEGER:
  {
    data.type = DataType::UNSIGNED_INTEGER;
    data.value.unsigned_integer_value = (uint32_t)atoi(data_value_string);
    break;
  }
  case DataType::UNSIGNED_LONG:
  {
    data.type = DataType::UNSIGNED_LONG;
    data.value.unsigned_long_value = (uint64_t)atoi(data_value_string);
    break;
  }
  case DataType::SIGNED_SHORT:
  {
    data.type = DataType::SIGNED_SHORT;
    data.value.signed_short_value = (uint8_t)atoi(data_value_string);
    break;
  }
  case DataType::SIGNED_INTEGER:
  {
    data.type = DataType::SIGNED_INTEGER;
    data.value.signed_integer_value = (int32_t)atoi(data_value_string);
    break;
  }
  case DataType::SIGNED_LONG:
  {
    data.type = DataType::SIGNED_LONG;
    data.value.signed_long_value = (int64_t)atoi(data_value_string);
    break;
  }
  case DataType::DOUBLE:
  {
    data.type = DataType::DOUBLE;
    data.value.double_value = atof(data_value_string);
    break;
  }
  case DataType::BOOLEAN:
  {
    data.type = DataType::BOOLEAN;
    data.value.boolean_value = identifyBoolean(data_value_string);
    break;
  }
  case DataType::STRING:
  {
    data.type = DataType::STRING;
    copyCharArray(data_value_string, data.value.string_value);
    break;
  }
  case DataType::UNKNOWN:
  {
    data.type = DataType::UNKNOWN;
    break;
  }
  }
  return data;
}