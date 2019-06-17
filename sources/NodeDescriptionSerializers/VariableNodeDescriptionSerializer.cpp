#include "DataValueIdentifier.hpp"
#include "JsonSerializerHeader.hpp"

using nlohmann::json;

void to_json(json &j, const VariableNodeDescription &node) {
  j = json{{"dataType", node.data.type}};
}

void from_json(const json &j, VariableNodeDescription &node) {
  j.at("dataType").get_to(node.data.type);
  node.data = identifyDataValue(j.at("dataType").get<DataType>(),
                                j.at("dataValue").get<std::string>().c_str());
}