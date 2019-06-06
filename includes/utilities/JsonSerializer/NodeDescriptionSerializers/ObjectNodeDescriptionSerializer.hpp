#ifndef _OBJECT_NODE_DESCRIPTION_SERIALIZER_HPP_
#define _OBJECT_NODE_DESCRIPTION_SERIALIZER_HPP_

#include "../JsonSerializerHeader.hpp"
namespace nlohmann
{
void to_json(json &j, const ObjectNodeDescription &node)
{
    j = json{
        {"childrenCount", node.children_count}};
}

void from_json(const json &j, ObjectNodeDescription &node)
{
    j.at("childrenCount").get_to(node.children_count);
}
} // namespace nlohmann
#endif //_OBJECT_NODE_DESCRIPTION_SERIALIZER_HPP_