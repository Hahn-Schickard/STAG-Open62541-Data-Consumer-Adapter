#include "Utility.hpp"

#include <open62541/types.h>
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
  case DataType::TIME: {
    return UA_TYPES[UA_TYPES_DATETIME].typeId;
  }
  case DataType::Unknown:
  default: {
    throw runtime_error("Could not convert STAG Information Model Data Type to "
                        "OPC UA data type!");
  }
  }
}

string toString(const UA_String* input) {
  string result = string((char*)(input->data), input->length);
  return result;
}

string toString(const UA_NodeId* node_id) {
  UA_String ua_string = UA_STRING_NULL;
  if (UA_NodeId_print(node_id, &ua_string) != UA_STATUSCODE_GOOD) {
    throw runtime_error("Failed to conver UA_NodeId to a string!");
  }
  auto ret = toString(&ua_string);
  UA_String_clear(&ua_string);
  return ret;
}

string toString(const UA_QualifiedName* name) {
  string result = to_string(name->namespaceIndex) + ":" + toString(&name->name);
  return result;
}

string toString(const UA_ExpandedNodeId& id) {
  return to_string(id.serverIndex) + ":" + toString(&id.namespaceUri) + ":" +
      toString(&id.nodeId);
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

StatusCodeNotGood::StatusCodeNotGood(
    const string& msg, const UA_StatusCode& code)
    : runtime_error("Received status code: " +
          string(UA_StatusCode_name(code)) + (msg.empty() ? "" : " " + msg)),
      status(code) {}

void checkStatusCode(
    const string& msg, const UA_StatusCode& status, bool uncertain_is_bad) {
  if (UA_StatusCode_isBad(status) ||
      (UA_StatusCode_isUncertain(status) && uncertain_is_bad)) {
    throw StatusCodeNotGood(msg, status);
  }
}

void checkStatusCode(const UA_StatusCode& status, bool uncertain_is_bad) {
  checkStatusCode(string(), status, uncertain_is_bad);
}

UA_Variant toUAVariant(const DataVariant& variant) {
  UA_Variant result;
  match(
      variant,
      [&result](bool value) {
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
      },
      [&result](uintmax_t value) {
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_uintmax]);
      },
      [&result](intmax_t value) {
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_intmax]);
      },
      [&result](double value) {
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
      },
      [&result](const DateTime& value) {
        UA_DateTime date_time = UA_DateTime_fromUnixTime(value.getValue());
        UA_Variant_setScalarCopy(
            &result, &date_time, &UA_TYPES[UA_TYPES_DATETIME]);
      },
      [&result](const string& value) {
        auto ua_string = makeUAString(value);
        UA_Variant_setScalarCopy(
            &result, &ua_string, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&ua_string);
      },
      [&result](const vector<uint8_t>& value) {
        auto ua_byte_string = makeUAByteString(value);
        UA_Variant_setScalarCopy(
            &result, &ua_byte_string, &UA_TYPES[UA_TYPES_BYTESTRING]);
        UA_String_clear(&ua_byte_string);
      });
  return result;
}

DataVariant toDataVariant(const UA_Variant& variant) {
  switch (variant.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    bool value = *((bool*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    // NOLINTNEXTLINE(bugprone-signed-char-misuse,cert-str34-c)
    intmax_t value = *((UA_SByte*)(variant.data)); // this is not a char
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    intmax_t value = *((UA_Int16*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    intmax_t value = *((UA_Int32*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    intmax_t value = *((UA_Int64*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    UA_DateTime time_value = *((UA_DateTime*)(variant.data));
    DateTime value(UA_DateTime_toUnixTime(time_value));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    uintmax_t value = *((UA_Byte*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    uintmax_t value = *((UA_UInt16*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    uintmax_t value = *((UA_UInt32*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    uintmax_t value = *((UA_UInt64*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    UA_StatusCode status_code = *((UA_StatusCode*)(variant.data));
    auto value = string(UA_StatusCode_name(status_code));
    if (!value.empty()) {
      return DataVariant(value);
    } else {
      // if no human readable string is available, return the code as an integer
      return DataVariant(to_string(status_code));
    }
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    double value = *((UA_Float*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    double value = *((UA_Double*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    auto* byte_string = (UA_ByteString*)(variant.data);
    auto value = vector<uint8_t>(
        byte_string->data, byte_string->data + byte_string->length);
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    auto* ua_string = (UA_String*)(variant.data);
    auto value = toString(ua_string);
    return DataVariant(value);
  }
  default: {
    string error_msg = string(variant.type->typeName) +
        " into Information_Model::DataVariant conversion is not supported";
    throw runtime_error(error_msg);
  }
  }
}
} // namespace open62541