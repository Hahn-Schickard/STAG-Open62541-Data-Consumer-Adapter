#ifndef _OBJECT_NODE_DESCRIPTION_SERIALIZER_HPP_
#define _OBJECT_NODE_DESCRIPTION_SERIALIZER_HPP_

#include "../JsonSerializerHeader.hpp"

using json = nlohmann::json;

void to_json(json &j, const ObjectNodeDescription &node)
{
    j = json{
        {"childrenCount", node.childrenCount}};
}

void from_json(const json &j, ObjectNodeDescription &node)
{
    j.at("childrenCount").get_to(node.childrenCount);
}

#endif //_OBJECT_NODE_DESCRIPTION_SERIALIZER_HPP_