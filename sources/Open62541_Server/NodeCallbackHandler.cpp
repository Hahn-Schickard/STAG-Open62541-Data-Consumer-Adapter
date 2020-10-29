#include "NodeCallbackHandler.hpp"
#include "Utility.hpp"
#include "Variant_Visitor.hpp"

using namespace std;
using namespace HaSLL;
using namespace Information_Model;

bool operator==(const UA_NodeId &lhs, const UA_NodeId &rhs) {
  return UA_NodeId_equal(&lhs, &rhs);
}

namespace open62541 {
CallbackWrapper::CallbackWrapper()
    : CallbackWrapper(DataType::UNKNOWN, nullptr, nullptr) {}

CallbackWrapper::CallbackWrapper(DataType type, ReadCallback read_callback)
    : CallbackWrapper(type, read_callback, nullptr) {}

CallbackWrapper::CallbackWrapper(DataType type, ReadCallback read_callback,
                                 WriteCallback write_callback)
    : data_type_(type) {
  if (read_callback) {
    readable_ = read_callback;
  }
  if (write_callback) {
    writable_ = write_callback;
  } else {
    writable_ = nullopt;
  }
}

void NodeCallbackHandler::initialise(const UA_Logger *logger) {
  logger_ = logger;
}

void NodeCallbackHandler::destroy() { node_calbacks_map_.clear(); }

NodeCallbackHandler::NodeCalbackMap::iterator
NodeCallbackHandler::findIndexPosition(const UA_NodeId *node_id) {
  NodeCalbackMap::iterator it;
  it = node_calbacks_map_.find(*node_id);
  return it;
}

CallbackWrapperPtr
NodeCallbackHandler::findCallbackWrapper(const UA_NodeId *node_id) {
  auto result = make_shared<CallbackWrapper>();
  auto it = findIndexPosition(node_id);
  if (it != node_calbacks_map_.end()) {
    string trace_msg = "Node " + toString(node_id) + "callback_wrapper found.";
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
    result = it->second;
  } else {
    string error_msg =
        "Node " + toString(node_id) + " does not have any callback registered!";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
  }
  return result;
}

UA_StatusCode
NodeCallbackHandler::addNodeCallbacks(UA_NodeId node_id,
                                      CallbackWrapperPtr callback_wrapper) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  if (findCallbackWrapper(&node_id)) {
    string trace_msg = "Adding callback_wrapper for Node " + toString(&node_id);
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
    node_calbacks_map_.emplace(move(node_id), move(callback_wrapper));
    status = UA_STATUSCODE_GOOD;
  } else {
    string error_msg =
        "Node " + toString(&node_id) + " was already registered ealier";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
    status = UA_STATUSCODE_BADNODEIDEXISTS;
  }
  return status;
}

UA_StatusCode
NodeCallbackHandler::removeNodeCallbacks(const UA_NodeId *node_id) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  auto it = findIndexPosition(node_id);
  if (it != node_calbacks_map_.end()) {
    string trace_msg = "Removing callbacks for Node " + toString(node_id);
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
    node_calbacks_map_.erase(it);
    status = UA_STATUSCODE_GOOD;
  } else {
    string error_msg =
        "Node " + toString(node_id) + " does not have any callback registered!";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
    status = UA_STATUSCODE_BADNOTFOUND;
  }
  return status;
}

UA_StatusCode NodeCallbackHandler::readNodeValue(
    UNUSED(UA_Server *server), UNUSED(const UA_NodeId *sessionId),
    UNUSED(void *sessionContext), const UA_NodeId *node_id,
    UNUSED(void *nodeContext), UNUSED(UA_Boolean includeSourceTimeStamp),
    UNUSED(const UA_NumericRange *range), UA_DataValue *value) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTREADABLE;

  auto it = findIndexPosition(node_id);
  if (it != node_calbacks_map_.end()) {
    string trace_msg = "Calling read callback for Node " + toString(node_id);
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
    auto callback_wrapper = it->second;
    try {
      auto variant_value = callback_wrapper->readable_();
      match(variant_value,
            [&](bool boolean_value) {
              if (callback_wrapper->data_type_ == DataType::BOOLEAN) {
                UA_Variant_setScalarCopy(&value->value, &boolean_value,
                                         &UA_TYPES[UA_TYPES_BOOLEAN]);
              } else {
                throw runtime_error("Tried to read a Boolean data type "
                                    "when node data type is: " +
                                    toString(callback_wrapper->data_type_));
              }
            },
            [&](uint64_t integer_value) {
              if (callback_wrapper->data_type_ == DataType::UNSIGNED_INTEGER) {
                UA_Variant_setScalarCopy(&value->value, &integer_value,
                                         &UA_TYPES[UA_TYPES_INT32]);
              } else {
                throw runtime_error("Tried to read a Integer data type "
                                    "when node data type is:" +
                                    toString(callback_wrapper->data_type_));
              }
            },
            [&](int64_t long_value) {
              if (callback_wrapper->data_type_ == DataType::INTEGER) {
                UA_Variant_setScalarCopy(&value->value, &long_value,
                                         &UA_TYPES[UA_TYPES_INT64]);
              } else {
                throw runtime_error("Tried to read a Long data type "
                                    "when node data type is: " +
                                    toString(callback_wrapper->data_type_));
              }
            },
            [&](double double_value) {
              if (callback_wrapper->data_type_ == DataType::DOUBLE) {
                UA_Variant_setScalarCopy(&value->value, &double_value,
                                         &UA_TYPES[UA_TYPES_DOUBLE]);
              } else {
                throw runtime_error("Tried to read a Double data type "
                                    "when node data type is:" +
                                    toString(callback_wrapper->data_type_));
              }
            },
            [&](vector<uint8_t> opaque_value) {
              if (callback_wrapper->data_type_ == DataType::OPAQUE) {
                string tmp(opaque_value.begin(), opaque_value.end());
                auto byte_string = UA_BYTESTRING_ALLOC(tmp.c_str());
                UA_Variant_setScalarCopy(&value->value, &byte_string,
                                         &UA_TYPES[UA_TYPES_BYTESTRING]);
              } else {
                throw runtime_error("Tried to read a String data type "
                                    "when node data type is: " +
                                    toString(callback_wrapper->data_type_));
              }
            },
            [&](string string_value) {
              if (callback_wrapper->data_type_ == DataType::STRING) {
                UA_String open62541_string;
                open62541_string.length = strlen(string_value.c_str());
                open62541_string.data =
                    (UA_Byte *)malloc(open62541_string.length);
                memcpy(open62541_string.data, string_value.c_str(),
                       open62541_string.length);
                UA_Variant_setScalarCopy(&value->value, &open62541_string,
                                         &UA_TYPES[UA_TYPES_STRING]);
              } else {
                throw runtime_error("Tried to read a String data type "
                                    "when node data type is: " +
                                    toString(callback_wrapper->data_type_));
              }
            });
      value->hasValue = true;
      status = UA_STATUSCODE_GOOD;
    } catch (const runtime_error &error) {
      string error_msg = "Type missmatch error occured while reading Node " +
                         toString(node_id) + " Error message: " + error.what();
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
      status = UA_STATUSCODE_BADTYPEMISMATCH;
    } catch (const exception &exp) {
      string error_msg = "An exception occoured, while reading Node " +
                         toString(node_id) + " exception: " + exp.what();
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
      status = UA_STATUSCODE_BADINTERNALERROR;
    }
  } else {
    string error_msg = "Node " + toString(node_id) +
                       " does not have any registered read callbacks!";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
  }
  return status;
}

UA_StatusCode NodeCallbackHandler::writeNodeValue(
    UNUSED(UA_Server *server), UNUSED(const UA_NodeId *sessionId),
    UNUSED(void *sessionContext), const UA_NodeId *node_id,
    UNUSED(void *nodeContext), UNUSED(const UA_NumericRange *range),
    const UA_DataValue *value) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTWRITABLE;
  auto it = findIndexPosition(node_id);
  if (it != node_calbacks_map_.end()) {
    auto callback_wrapper = it->second;
    if (it->second->writable_.has_value()) {
      string trace_msg = "Calling write callback for Node " + toString(node_id);
      UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
      auto write_CB = callback_wrapper->writable_.value();
      switch (value->value.type->typeKind) {
      case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
        bool boolean_value = *((bool *)(value->value.data));
        write_CB(DataVariant(boolean_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE:
      case UA_DataTypeKind::UA_DATATYPEKIND_INT16:
      case UA_DataTypeKind::UA_DATATYPEKIND_INT32:
      case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
        int64_t long_value = *((int64_t *)(value->value.data));
        write_CB(DataVariant(long_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME:
      case UA_DataTypeKind::UA_DATATYPEKIND_BYTE:
      case UA_DataTypeKind::UA_DATATYPEKIND_UINT16:
      case UA_DataTypeKind::UA_DATATYPEKIND_UINT32:
      case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
        uint64_t long_value = *((uint64_t *)(value->value.data));
        write_CB(DataVariant(long_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT:
      case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
        double double_value = *((double *)(value->value.data));
        write_CB(DataVariant(double_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING:
      case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE:
      case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
        UA_String *ua_string = (UA_String *)(value->value.data);
        auto string_value = string((char *)ua_string->data, ua_string->length);
        write_CB(DataVariant(string_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_GUID: {
        UA_Guid ua_guid;
        ua_guid.data1 = ((UA_Guid *)(value->value.data))->data1;
        ua_guid.data2 = ((UA_Guid *)(value->value.data))->data2;
        ua_guid.data3 = ((UA_Guid *)(value->value.data))->data3;
        for (uint8_t i = 0; i < 8; i++) {
          ua_guid.data4[i] = ((UA_Guid *)(value->value.data))->data4[i];
        }
        auto string_value = toString(ua_guid);
        write_CB(DataVariant(string_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      default: {
        string error_msg = "Node " + toString(node_id) + " does not take " +
                           string(value->value.type->typeName) +
                           "as its argument for write callback!";
        UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
        status = UA_STATUSCODE_BADINVALIDARGUMENT;
        break;
      }
      }
    } else {
      string error_msg = "Node " + toString(node_id) +
                         " does not have any registered write callback!";
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
    }
  } else {
    string error_msg = "Node " + toString(node_id) +
                       " does not have any registered callbacks!";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
    status = UA_STATUSCODE_BADNOTFOUND;
  }
  return status;
}

unordered_map<UA_NodeId, CallbackWrapperPtr, UA_NodeId_Hasher>
    NodeCallbackHandler::node_calbacks_map_;   // NOLINT
const UA_Logger *NodeCallbackHandler::logger_; // NOLINT
} // namespace open62541