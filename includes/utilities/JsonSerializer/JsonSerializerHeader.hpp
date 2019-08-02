#ifndef _JSON_SERIALIZER_HEADER_HPP_
#define _JSON_SERIALIZER_HEADER_HPP_

#include "CStringFormater.hpp"
#include "NodeInformation.h"
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

NLOHMANN_JSON_SERIALIZE_ENUM(NodeClassType, {
                                                {VARIABLE_NODE, "variable"},
                                                {METHOD_NODE, "method"},
                                                {OBJECT_NODE, "object"},
                                                {UNRECOGNIZED_NODE, nullptr},
                                            })

NLOHMANN_JSON_SERIALIZE_ENUM(DataType,
                             {
                                 {UNSIGNED_SHORT, "Unsigned Short"},
                                 {UNSIGNED_INTEGER, "Unsigned Integer"},
                                 {UNSIGNED_LONG, "Unsigned Long"},
                                 {SIGNED_SHORT, "Signed Short"},
                                 {SIGNED_INTEGER, "Signed Integer"},
                                 {SIGNED_LONG, "Signed Long"},
                                 {DOUBLE, "Double"},
                                 {BOOLEAN, "Boolean"},
                                 {STRING, "String"},
                                 {UNKNOWN, nullptr},
                             })

void to_json(json &j, const BaseNodeDescription &node);
void from_json(const json &j, BaseNodeDescription &node);

void to_json(json &j, const VariableNodeDescription &node);
void from_json(const json &j, VariableNodeDescription &node);

void to_json(json &j, const ObjectNodeDescription &node);
void from_json(const json &j, ObjectNodeDescription &node);

void to_json(json &j, const NodeDescription &node);
void from_json(const json &j, NodeDescription &node);

#endif // JSON_SERIALIZER_HEADER_HPP_utilities