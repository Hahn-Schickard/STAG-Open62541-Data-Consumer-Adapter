#include "DataValueIdentifier.hpp"
#include "JsonSerializerHeader.hpp"

using nlohmann::json;

void to_json(json &j, const VariableNodeDescription &node) {
  j = json{{"dataType", node.data_type}};
}

void from_json(const json &j, VariableNodeDescription &node) {
  j.at("dataType").get_to(node.data_type);
  node.data_value = identifyDataValue(
      node.data_type, j.at("dataValue").get<std::string>().c_str());
}