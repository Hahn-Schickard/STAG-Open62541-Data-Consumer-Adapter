#ifndef _NODE_DESCRIPTION_SERIALIZER_HPP_
#define _NODE_DESCRIPTION_SERIALIZER_HPP_

#include "../JsonSerializerHeader.hpp"

namespace nlohmann
{
void to_json(json &j, const NodeDescription &node)
{
    switch (node.base_node_descriptor.node_class)
    {
    case NodeClassType::VARIABLE_NODE:
    {
        j = json{
            {"baseNodeDescriptor",
             {"browserName", node.base_node_descriptor.browse_name},
             {"description", node.base_node_descriptor.description},
             {"locale", node.base_node_descriptor.locale},
             {"uniqueId", node.base_node_descriptor.unique_id},
             {"displayName", node.base_node_descriptor.display_name},
             {"writableFlag", node.base_node_descriptor.writable_flag},
             {"nodeClass", node.base_node_descriptor.node_class}},
            {"variableNodeDescriptor",
             {"dataType", node.variable_node_descriptor.data_type},
             {"dataValue", nullptr}}};
        break;
    }
    case NodeClassType::METHOD_NODE:
    {
        // TODO: handle method serialization
        j = json{
            {"baseNodeDescriptor",
             {"browserName", node.base_node_descriptor.browse_name},
             {"description", node.base_node_descriptor.description},
             {"locale", node.base_node_descriptor.locale},
             {"uniqueId", node.base_node_descriptor.unique_id},
             {"displayName", node.base_node_descriptor.display_name},
             {"writableFlag", node.base_node_descriptor.writable_flag},
             {"nodeClass", node.base_node_descriptor.node_class}}};
        break;
    }
    case NodeClassType::OBJECT_NODE:
    {
        j = json{
            {"baseNodeDescriptor",
             {"browserName", node.base_node_descriptor.browse_name},
             {"description", node.base_node_descriptor.description},
             {"locale", node.base_node_descriptor.locale},
             {"uniqueId", node.base_node_descriptor.unique_id},
             {"displayName", node.base_node_descriptor.display_name},
             {"writableFlag", node.base_node_descriptor.writable_flag},
             {"nodeClass", node.base_node_descriptor.node_class}},
            {"objectNodeDescriptor",
             {"childrenCount", node.object_node_descriptor.children_count}}};
        break;
    }
    default:
    {
        break;
    }
    }
}

void from_json(const json &j, NodeDescription &node)
{
    copyCharArray(j.at("baseNodeDescriptor").at("browseName").get<std::string>().c_str(),
                  node.base_node_descriptor.browse_name);
    copyCharArray(j.at("baseNodeDescriptor").at("description").get<std::string>().c_str(),
                  node.base_node_descriptor.description);
    copyCharArray(j.at("baseNodeDescriptor").at("displayName").get<std::string>().c_str(),
                  node.base_node_descriptor.display_name);
    copyCharArray(j.at("baseNodeDescriptor").at("locale").get<std::string>().c_str(),
                  node.base_node_descriptor.locale);
    copyCharArray(j.at("baseNodeDescriptor").at("uniqueId").get<std::string>().c_str(),
                  node.base_node_descriptor.unique_id);
    j.at("baseNodeDescriptor").at("writableFlag").get_to(node.base_node_descriptor.writable_flag);
    j.at("baseNodeDescriptor").at("nodeClass").get_to(node.base_node_descriptor.node_class);

    switch (node.base_node_descriptor.node_class)
    {
    case NodeClassType::VARIABLE_NODE:
    {
        j.at("variableNodeDescriptor").at("dataType").get_to(node.variable_node_descriptor.data_type);
        node.variable_node_descriptor.data_value = identifyDataValue(node.variable_node_descriptor.data_type,
                                                                     j.at("variableNodeDescriptor").at("dataValue").get<std::string>().c_str());
        break;
    }
    case NodeClassType::METHOD_NODE:
    {
        // TODO: handle method serialization
        break;
    }
    case NodeClassType::OBJECT_NODE:
    {
        j.at("objectNodeDescriptor").at("childrenCount").get_to(node.object_node_descriptor.children_count);
        break;
    }
    default:
    {
        break;
    }
    }
}
} // namespace nlohmann

#endif //_NODE_DESCRIPTION_SERIALIZER_HPP_