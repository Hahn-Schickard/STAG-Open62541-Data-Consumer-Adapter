#include "StringConverter.hpp"

#include <stdexcept>

namespace open62541 {
using namespace std;

string toString(const UA_String* input) {
  auto result = string((char*)(input->data), input->length);
  return result;
}

string toString(const UA_NodeId* node_id) {
  auto ua_string = UA_STRING_NULL;
  if (UA_NodeId_print(node_id, &ua_string) != UA_STATUSCODE_GOOD) {
    throw runtime_error("Failed to conver UA_NodeId to a string!");
  }
  auto ret = toString(&ua_string);
  UA_String_clear(&ua_string);
  return ret;
}

string toString(const UA_QualifiedName* name) {
  auto result = to_string(name->namespaceIndex) + ":" + toString(&name->name);
  return result;
}

string toString(const UA_ExpandedNodeId& id) {
  return to_string(id.serverIndex) + ":" + toString(&id.namespaceUri) + ":" +
      toString(&id.nodeId);
}

string toString(UA_DateTime timestamp) {
  auto calendar_time = UA_DateTime_toStruct(timestamp);
  /* Format UA_DateTime into a %Y-%m-%d %H:%M:%S.%ms%us*/
  auto result = to_string(calendar_time.year) + "-" +
      to_string(calendar_time.month) + "-" + to_string(calendar_time.day) +
      " " + to_string(calendar_time.hour) + ":" + to_string(calendar_time.min) +
      ":" + to_string(calendar_time.sec) + "." +
      to_string(calendar_time.milliSec) + to_string(calendar_time.microSec);
  return result;
}

UA_String makeUAString(const string& input) {
  UA_String result;
  result.length = strlen(input.c_str());
  result.data = (UA_Byte*)malloc(result.length);
  memcpy(result.data, input.c_str(), result.length);
  return result;
}

UA_ByteString makeUAByteString(const vector<uint8_t>& input) {
  UA_ByteString result;
  result.length = input.size();
  result.data = (UA_Byte*)malloc(result.length);
  memcpy(result.data, input.data(), result.length);
  return result;
}

string toSqlType(const UA_DataType* variant) {
  switch (variant->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    return "BOOLEAN";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    return "OPCUA_TINYUINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    return "OPCUA_TINYINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    return "OPCUA_SMALLUINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    return "SMALLINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    return "OPCUA_UINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    return "INT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    return "OPCUA_BIGUINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    return "BIGINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    return "OPCUA_STATUSCODE";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    return "REAL";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    return "DOUBLE PRECISION";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    return "TIMESTAMP(6)";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    return "BYTEA";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    return "TEXT";
  }
  // NOLINTNEXTLINE(bugprone-branch-clone)
  case UA_DataTypeKind::UA_DATATYPEKIND_GUID: {
    [[fallthrough]];
  }
  default: {
    string error_msg =
        "Unhandeled UA_DataType detected: " + string(variant->typeName);
    throw logic_error(error_msg);
  }
  }
}
} // namespace open62541
