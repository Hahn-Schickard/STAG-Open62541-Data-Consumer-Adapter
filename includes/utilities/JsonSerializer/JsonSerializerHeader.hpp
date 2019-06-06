#ifndef _JSON_SERIALIZER_HEADER_HPP
#define _JSON_SERIALIZER_HEADER_HPP

#include "NodeInformation.h"
#include "CStringFormater.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

namespace nlohmann
{
NLOHMANN_JSON_SERIALIZE_ENUM(NodeClassType, {
                                                {VARIABLE_NODE, "variable"},
                                                {METHOD_NODE, "method"},
                                                {OBJECT_NODE, "object"},
                                                {UNRECOGNIZED_NODE, nullptr},
                                            })

NLOHMANN_JSON_SERIALIZE_ENUM(DataType, {
                                           {UNSIGNED_SHORT, "Unsigned Short"},
                                           {UNSIGNED_INTEGER, "Unsigned Integer"},
                                           {UNSIGNED_LONG, "Unsigned Long"},
                                           {SIGNED_SHORT, "Signed Short"},
                                           {SIGNED_INTEGER, "Signed Integer"},
                                           {SIGNED_LONG, "Signed Long"},
                                           {DOUBLE, "Double"},
                                           {BOOLEAN, "Boolean"},
                                           {STRING, "String"},
                                           {UNKNOWN, nullptr},
                                       })

DataValue identifyDataValue(DataType data_type, const char *data_value_string)
{
    DataValue data_value;

    switch (data_type)
    {
    case DataType::UNSIGNED_SHORT:
    {
        data_value.unsigned_short_value = (uint8_t)atoi(data_value_string);
        break;
    }
    case DataType::UNSIGNED_INTEGER:
    {
        data_value.unsigned_integer_value = (uint32_t)atoi(data_value_string);
        break;
    }
    case DataType::UNSIGNED_LONG:
    {
        data_value.unsigned_long_value = (uint64_t)atoi(data_value_string);
        break;
    }
    case DataType::SIGNED_SHORT:
    {
        data_value.signed_short_value = (uint8_t)atoi(data_value_string);
        break;
    }
    case DataType::SIGNED_INTEGER:
    {
        data_value.signed_integer_value = (int32_t)atoi(data_value_string);
        break;
    }
    case DataType::SIGNED_LONG:
    {
        data_value.signed_long_value = (int64_t)atoi(data_value_string);
        break;
    }
    case DataType::DOUBLE:
    {
        data_value.double_value = atof(data_value_string);
        break;
    }
    case DataType::BOOLEAN:
    {
        data_value.boolean_value = identifyBoolean(data_value_string);
        break;
    }
    case DataType::STRING:
    {
        copyCharArray(data_value_string, data_value.string_value);
        break;
    }
    case DataType::UNKNOWN:
    {
        break;
    }
    }
}
} // namespace nlohmann
#endif //JSON_SERIALIZER_HEADER_HPP