#include "JsonSerializerHeader.hpp"

using nlohmann::json;

void to_json(json &j, const BaseNodeDescription &node)
{
    j = json{
        {"browserName", node.browse_name},
        {"description", node.description},
        {"locale", node.locale},
        {"uniqueId", node.unique_id},
        {"displayName", node.display_name},
        {"writableFlag", node.writable_flag},
        {"nodeClass", node.node_class}};
}

void from_json(const json &j, BaseNodeDescription &node)
{
    copyCharArray(j.at("browseName").get<std::string>().c_str(), node.browse_name);
    copyCharArray(j.at("description").get<std::string>().c_str(), node.description);
    copyCharArray(j.at("displayName").get<std::string>().c_str(), node.display_name);
    copyCharArray(j.at("locale").get<std::string>().c_str(), node.locale);
    copyCharArray(j.at("uniqueId").get<std::string>().c_str(), node.unique_id);
    j.at("writableFlag").get_to(node.writable_flag);
    j.at("nodeClass").get_to(node.node_class);
}