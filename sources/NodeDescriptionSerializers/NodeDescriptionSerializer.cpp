#include "JsonSerializerHeader.hpp"

using nlohmann::json;

void to_json(json &j, const NodeDescription &node)
{
    switch (node.base_node_descriptor.node_class)
    {
    case NodeClassType::VARIABLE_NODE:
    {
        j = json{
            {"baseNodeDescriptor", node.base_node_descriptor},
            {"variableNodeDescriptor", node.variable_node_descriptor},
            {"objectNodeDescriptor", nullptr},
            {"methodNodeDescriptor", nullptr}};
        break;
    }
    case NodeClassType::METHOD_NODE:
    {
        // TODO: handle method serialization
        j = json{
            {"baseNodeDescriptor", node.base_node_descriptor},
            {"variableNodeDescriptor", nullptr},
            {"objectNodeDescriptor", nullptr},
            {"methodNodeDescriptor", nullptr}};
        break;
    }
    case NodeClassType::OBJECT_NODE:
    {
        j = json{
            {"baseNodeDescriptor", node.base_node_descriptor},
            {"variableNodeDescriptor", nullptr},
            {"objectNodeDescriptor", node.object_node_descriptor},
            {"methodNodeDescriptor", nullptr}};
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
    j.at("baseNodeDescriptor").get_to(node.base_node_descriptor);

    switch (node.base_node_descriptor.node_class)
    {
    case NodeClassType::VARIABLE_NODE:
    {
        j.at("variableNodeDescriptor").get_to(node.variable_node_descriptor);
        break;
    }
    case NodeClassType::METHOD_NODE:
    {
        // TODO: handle method serialization
        break;
    }
    case NodeClassType::OBJECT_NODE:
    {
        j.at("objectNodeDescriptor").get_to(node.object_node_descriptor);
        break;
    }
    default:
    {
        break;
    }
    }
}