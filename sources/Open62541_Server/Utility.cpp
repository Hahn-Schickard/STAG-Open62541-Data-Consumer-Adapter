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

StatusCodeNotGood::StatusCodeNotGood(
    const string msg, const UA_StatusCode& code)
    : runtime_error("Received status code: " +
          string(UA_StatusCode_name(code)) + (msg.empty() ? "" : " " + msg)),
      status(code) {}

void checkStatusCode(
    const std::string msg, const UA_StatusCode& status, bool uncertain_is_bad) {
  if (UA_StatusCode_isBad(status) ||
      (UA_StatusCode_isUncertain(status) && uncertain_is_bad)) {
    throw StatusCodeNotGood(msg, status);
  }
}

void checkStatusCode(const UA_StatusCode& status, bool uncertain_is_bad) {
  checkStatusCode(string(), status, uncertain_is_bad);
}

} // namespace open62541