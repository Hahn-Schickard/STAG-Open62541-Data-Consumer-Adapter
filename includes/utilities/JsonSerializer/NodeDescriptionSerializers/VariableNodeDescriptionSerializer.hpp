#ifndef _VARIABLE_NODE_DESCRIPTION_SERIALIZER_HPP_
#define _VARIABLE_NODE_DESCRIPTION_SERIALIZER_HPP_

#include "../JsonSerializerHeader.hpp"
namespace nlohmann
{
void to_json(json &j, const VariableNodeDescription &node)
{
    j = json{
        {"dataType", node.data_type},
        {"dataValue", nullptr}};
}

void from_json(const json &j, VariableNodeDescription &node)
{
    j.at("dataType").get_to(node.data_type);
    node.data_value = identifyDataValue(node.data_type, j.at("dataValue").get<std::string>().c_str());
}
} // namespace nlohmann
#endif //_BASE_NODE_DESCRIPTION_SERIALIZER_HPP_