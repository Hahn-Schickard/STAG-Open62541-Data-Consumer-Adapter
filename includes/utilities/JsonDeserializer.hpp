#ifndef _JSON_DESERIALIZER_HPP
#define _JSON_DESERIALIZER_HPP

#include "NodeInformation.h"
#include "CStringFormater.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

NLOHMANN_JSON_SERIALIZE_ENUM(NodeClassType, {
                                                {VARIABLE_NODE, "variable"},
                                                {METHOD_NODE, "method"},
                                                {OBJECT_NODE, "object"},
                                                {UNRECOGNIZED_NODE, nullptr},
                                            })

void to_json(json &j, const BaseNodeDescription &node)
{
    j = json{
        {"browserName", node.browseName},
        {"description", node.description},
        {"displayName", node.displayName},
        {"locale", node.locale},
        {"uniqueId", node.uniqueId},
        {"writableFlag", node.writableFlag},
        {"nodeClass", node.nodeClass}};
}

void from_json(const json &j, BaseNodeDescription &node)
{
    copyCharArray(j.at("browseName").get<std::string>().c_str(), node.browseName);
    copyCharArray(j.at("description").get<std::string>().c_str(), node.description);
    copyCharArray(j.at("displayName").get<std::string>().c_str(), node.displayName);
    copyCharArray(j.at("locale").get<std::string>().c_str(), node.locale);
    copyCharArray(j.at("uniqueId").get<std::string>().c_str(), node.uniqueId);
    j.at("writableFlag").get_to(node.writableFlag);
    j.at("nodeClass").get_to(node.nodeClass);
}

BaseNodeDescription deserializeBaseNodeDescriptor(nlohmann::json &jsonDescriptor)
{
    return jsonDescriptor.get<BaseNodeDescription>();
}

std::vector<BaseNodeDescription> setUpBaseNodeDescriptor(std::ifstream &fileStream)
{

    json jsonFile;
    fileStream >> jsonFile;
    std::vector<BaseNodeDescription> baseDescriptors = jsonFile;
    return baseDescriptors;
}

#endif //_JSON_DESERIALIZER_HPP