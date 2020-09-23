#include "Utility.hpp"

#include <open62541/types_generated.h>

using namespace std;
using namespace Information_Model;

namespace open62541 {

UA_NodeId toNodeId(DataType type) {
  UA_NodeId typeId;
  switch (type) {
  case BOOLEAN: {
    typeId = UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    break;
  }
  case BYTE: {
    typeId = UA_TYPES[UA_TYPES_BYTE].typeId;
    break;
  }
  case SHORT: {
    typeId = UA_TYPES[UA_TYPES_INT16].typeId;
    break;
  }
  case INTEGER: {
    typeId = UA_TYPES[UA_TYPES_INT32].typeId;
    break;
  }
  case LONG: {
    typeId = UA_TYPES[UA_TYPES_INT64].typeId;
    break;
  }
  case FLOAT: {
    typeId = UA_TYPES[UA_TYPES_FLOAT].typeId;
    break;
  }
  case DOUBLE: {
    typeId = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    break;
  }
  case STRING: {
    typeId = UA_TYPES[UA_TYPES_STRING].typeId;
    break;
  }
  case UNKNOWN:
  default: {
    //@TODO: Log unknown data type declarations
    break;
  }
  }
  return typeId;
}

string toString(DataType type) {
  switch (type) {
  case DataType::BOOLEAN: {
    return "BOOLEAN";
  }
  case DataType::BYTE: {
    return "BYTE";
  }
  case DataType::SHORT: {
    return "SHORT";
  }
  case DataType::INTEGER: {
    return "INTEGER";
  }
  case DataType::LONG: {
    return "LONG";
  }
  case DataType::FLOAT: {
    return "FLOAT";
  }
  case DataType::DOUBLE: {
    return "DOUBLE";
  }
  case DataType::STRING: {
    return "STRING";
  }
  case DataType::UNKNOWN:
  default: { return "UNKNOWN"; }
  }
}

string toString(UA_String input) {
  string output;
  for (size_t i = 0; i < input.length; i++) {
    output.push_back(*input.data);
  }
  return output;
}

string toString(UA_Guid input) {
  string output = to_string(input.data1) + ":" + to_string(input.data2) + ":" +
                  to_string(input.data3) + ":";
  for (int i = 0; i < 8; i++) {
    output.push_back(input.data4[i]);
  }
  return output;
}

string toString(const UA_NodeId *nodeId) {
  string namespace_index_string = to_string(nodeId->namespaceIndex);
  string node_id_string;
  switch (nodeId->identifierType) {
  case UA_NodeIdType::UA_NODEIDTYPE_NUMERIC: {
    node_id_string = to_string(nodeId->identifier.numeric);
    break;
  }
  case UA_NodeIdType::UA_NODEIDTYPE_GUID: {
    node_id_string = toString(nodeId->identifier.guid);
    break;
  }
  case UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING:
  case UA_NodeIdType::UA_NODEIDTYPE_STRING: {
    node_id_string = toString(nodeId->identifier.string);
    break;
  }
  default: { node_id_string = "UNRECOGNIZED"; }
  }
  return namespace_index_string + ":" + node_id_string;
}

} // namespace open62541