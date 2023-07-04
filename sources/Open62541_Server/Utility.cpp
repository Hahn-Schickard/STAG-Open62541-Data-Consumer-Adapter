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
  case DataType::TIME: {
    return UA_TYPES[UA_TYPES_DATETIME].typeId;
  }
  case DataType::UNKNOWN:
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
  return toString(&ua_string);
}

string toString(const UA_QualifiedName* name) {
  string result = to_string(name->namespaceIndex) + ":" + toString(&name->name);
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
} // namespace open62541