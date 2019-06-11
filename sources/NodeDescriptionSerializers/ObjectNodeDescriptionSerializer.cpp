#include "JsonSerializerHeader.hpp"

using nlohmann::json;

void to_json(json &j, const ObjectNodeDescription &node) {
  for (int i = 0; i < node.children_count; i++) {
    j = json{{"childrenCount", node.children_count},
             {"children", node.children[i]}};
  }
}

void from_json(const json &j, ObjectNodeDescription &node) {
  j.at("childrenCount").get_to(node.children_count);

  if (node.children_count > 0) {
    node.children = (NodeDescription *)malloc(node.children_count);

    std::vector<json> children = j["children"];
    for (const auto child : children) {
      auto descriptor = child.at("nodeDescriptor").get<NodeDescription>();
      node.children = &descriptor;
      node.children++;
    }
  }
}