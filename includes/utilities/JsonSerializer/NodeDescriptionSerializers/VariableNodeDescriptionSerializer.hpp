#ifndef _VARIABLE_NODE_DESCRIPTION_SERIALIZER_HPP
#define _VARIABLE_NODE_DESCRIPTION_SERIALIZER_HPP

#include "../JsonSerializerHeader.hpp"

using json = nlohmann::json;

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

DataValue identifyDataValue(DataType dataType, const char *dataValueString)
{
    DataValue dataValue;

    switch (dataType)
    {
    case DataType::UNSIGNED_SHORT:
    {
        dataValue.unsigned_short_value = (uint8_t)atoi(dataValueString);
        break;
    }
    case DataType::UNSIGNED_INTEGER:
    {
        dataValue.unsigned_integer_value = (uint32_t)atoi(dataValueString);
        break;
    }
    case DataType::UNSIGNED_LONG:
    {
        dataValue.unsigned_long_value = (uint64_t)atoi(dataValueString);
        break;
    }
    case DataType::SIGNED_SHORT:
    {
        dataValue.signed_short_value = (uint8_t)atoi(dataValueString);
        break;
    }
    case DataType::SIGNED_INTEGER:
    {
        dataValue.signed_integer_value = (int32_t)atoi(dataValueString);
        break;
    }
    case DataType::SIGNED_LONG:
    {
        dataValue.signed_long_value = (int64_t)atoi(dataValueString);
        break;
    }
    case DataType::DOUBLE:
    {
        dataValue.double_value = atof(dataValueString);
        break;
    }
    case DataType::BOOLEAN:
    {
        dataValue.boolean_value = identifyBoolean(dataValueString);
        break;
    }
    case DataType::STRING:
    {
        copyCharArray(dataValueString, dataValue.string_value);
        break;
    }
    case DataType::UNKNOWN:
    {
        break;
    }
    }
}

void to_json(json &j, const VariableNodeDescription &node)
{
    j = json{
        {"dataType", node.dataType},
        {"dataValue", nullptr}};
}

void from_json(const json &j, VariableNodeDescription &node)
{
    j.at("dataType").get_to(node.dataType);
    node.dataValue = identifyDataValue(node.dataType, j.at("dataValue").get<std::string>().c_str());
}

#endif //_BASE_NODE_DESCRIPTION_SERIALIZER_HPP