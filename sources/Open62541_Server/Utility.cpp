#include "Utility.hpp"

#include <open62541/types_generated.h>
#include <stdexcept>

using namespace std;
using namespace Information_Model;

namespace open62541 {

UA_NodeId toNodeId(DataType type) {
  switch (type) {
  case DataType::BOOLEAN: {
    return UA_TYPES[UA_TYPES_BOOLEAN].typeId;
  }
  case DataType::UNSIGNED_INTEGER: {
    return UA_TYPES[UA_TYPES_UINT64].typeId;
  }
  case DataType::INTEGER: {
    return UA_TYPES[UA_TYPES_INT64].typeId;
  }
  case DataType::DOUBLE: {
    return UA_TYPES[UA_TYPES_DOUBLE].typeId;
  }
  case DataType::OPAQUE: {
    return UA_TYPES[UA_TYPES_BYTESTRING].typeId;
  }
  case DataType::STRING: {
    return UA_TYPES[UA_TYPES_STRING].typeId;
  }
  case DataType::UNKNOWN:
  default: { throw runtime_error("Unknown data type!"); }
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