#include "NodeCallbackHandler.hpp"
#include "Utility.hpp"
#include "Variant_Visitor.hpp"

using namespace std;
using namespace Information_Model;

bool operator==(const UA_NodeId& lhs, const UA_NodeId& rhs) {
  return UA_NodeId_equal(&lhs, &rhs);
}

namespace open62541 {
CallbackWrapper::CallbackWrapper(
    DataType type, const ReadCallback& read_callback)
    : data_type_(type), readable_(read_callback) {}

CallbackWrapper::CallbackWrapper(
    DataType type, const WriteCallback& write_callback)
    : data_type_(type), writable_(write_callback) {}

CallbackWrapper::CallbackWrapper(DataType type,
    const ReadCallback& read_callback, const WriteCallback& write_callback)
    : data_type_(type), readable_(read_callback), writable_(write_callback) {}

CallbackWrapper::CallbackWrapper(Information_Model::DataType type,
    const Function::ParameterTypes parameters,
    const ExecuteCallback& execute_callback)
    : data_type_(type), parameters_(parameters), executable_(execute_callback) {
}

CallbackWrapper::CallbackWrapper(Information_Model::DataType type,
    const Function::ParameterTypes parameters,
    const ExecuteCallback& execute_callback, const CallCallback& call_callback)
    : data_type_(type), parameters_(parameters), executable_(execute_callback),
      callable_(call_callback) {}

void NodeCallbackHandler::initialise(const UA_Logger* logger) {
  logger_ = logger;
}

void NodeCallbackHandler::destroy() { node_calbacks_map_.clear(); }

CallbackWrapperPtr NodeCallbackHandler::findCallbackWrapper(
    const UA_NodeId* node_id) {
  CallbackWrapperPtr result;
  auto it = node_calbacks_map_.find(*node_id);
  if (it != node_calbacks_map_.end()) {
    result = it->second;
  }
  return result;
}

UA_StatusCode NodeCallbackHandler::addNodeCallbacks(
    UA_NodeId node_id, const CallbackWrapperPtr& callback_wrapper) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  if (!callback_wrapper) {
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, "Precondition violated");
    status = UA_STATUSCODE_BADDEVICEFAILURE;
  } else if (!findCallbackWrapper(&node_id)) {
    string trace_msg = "Adding callback_wrapper for Node " + toString(&node_id);
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
    node_calbacks_map_.emplace(node_id, callback_wrapper);
    status = UA_STATUSCODE_GOOD;
  } else {
    string error_msg =
        "Node " + toString(&node_id) + " was already registered earlier";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
    status = UA_STATUSCODE_BADNODEIDEXISTS;
  }
  return status;
}

UA_StatusCode NodeCallbackHandler::removeNodeCallbacks(
    const UA_NodeId* node_id) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  auto it = node_calbacks_map_.find(*node_id);
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

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
UA_StatusCode NodeCallbackHandler::readNodeValue( // clang-format off
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* session_id,
    [[maybe_unused]] void* session_context, 
    const UA_NodeId* node_id,
    [[maybe_unused]] void* node_context,
    [[maybe_unused]] UA_Boolean include_source_time_stamp,
    [[maybe_unused]] const UA_NumericRange* range, 
    UA_DataValue* value) { // clang-format on
  UA_StatusCode status = UA_STATUSCODE_BADNOTREADABLE;

  auto it = node_calbacks_map_.find(*node_id);
  if (it != node_calbacks_map_.end()) {
    string trace_msg = "Calling read callback for Node " + toString(node_id);
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
    auto callback_wrapper = it->second;
    try {
      if (auto read = callback_wrapper->readable_) {
        string trace_msg =
            "Calling read callback for Node " + toString(node_id);
        UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
        auto data_variant = read();
        if (toDataType(data_variant) == callback_wrapper->data_type_) {
          value->value = toUAVariant(data_variant);
          value->hasValue = true;
          status = UA_STATUSCODE_GOOD;
        } else {
          string error_msg = "Expected to receive " +
              toString(callback_wrapper->data_type_) +
              " data type, but received " + toString(toDataType(data_variant)) +
              " instead while reading Node " + toString(node_id) + " value";
          UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
          status = UA_STATUSCODE_BADTYPEMISMATCH;
        }
      } else {
        string error_msg = "Node " + toString(node_id) +
            " does not have any registered read callback!";
        UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
      }
    } catch (const exception& exp) {
      string error_msg = "An exception occurred, while reading Node " +
          toString(node_id) + " exception: " + exp.what();
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
      status = UA_STATUSCODE_BADINTERNALERROR;
    }
  } else {
    string error_msg = "Node " + toString(node_id) +
        " does not have any registered read callbacks!";
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
    status = UA_STATUSCODE_BADNODEIDUNKNOWN;
  }
  return status;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
UA_StatusCode NodeCallbackHandler::writeNodeValue( // clang-format off
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* session_id,
    [[maybe_unused]] void* session_context, 
    const UA_NodeId* node_id,
    [[maybe_unused]] void* node_context,
    [[maybe_unused]] const UA_NumericRange* range, 
    const UA_DataValue* value) { // clang-format on
  UA_StatusCode status = UA_STATUSCODE_BADNOTWRITABLE;
  auto it = node_calbacks_map_.find(*node_id);
  if (it != node_calbacks_map_.end()) {
    auto callback_wrapper = it->second;
    if (callback_wrapper->writable_) {
      string trace_msg = "Calling write callback for Node " + toString(node_id);
      UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER, trace_msg.c_str());
      auto write_cb = callback_wrapper->writable_;
      auto data_variant = toDataVariant(value->value);
      try {
        write_cb(data_variant);
        status = UA_STATUSCODE_GOOD;
      } catch (const exception& ex) {
        string error_msg =
            "An unhandled exception occurred while trying to write to Node " +
            toString(node_id) + " Exception: " + ex.what();
        UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER, error_msg.c_str());
        status = UA_STATUSCODE_BADINTERNALERROR;
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
    status = UA_STATUSCODE_BADNODEIDUNKNOWN;
  }
  return status;
}

UA_StatusCode NodeCallbackHandler::callNodeMethod(UA_Server* server,
    const UA_NodeId* session_id, void* session_context,
    const UA_NodeId* method_id, void* method_context,
    const UA_NodeId* object_id, void* object_context, size_t input_size,
    const UA_Variant* input, size_t output_size, UA_Variant* output) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTWRITABLE;
  /**@TODO: implement callNodeMethod*/
  return status;
}

unordered_map<UA_NodeId, CallbackWrapperPtr, UA_NodeId_Hasher>
    NodeCallbackHandler::node_calbacks_map_; // NOLINT
const UA_Logger* NodeCallbackHandler::logger_; // NOLINT
} // namespace open62541