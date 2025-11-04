#include "NodeCallbackHandler.hpp"
#include "Utility.hpp"

#include <Variant_Visitor/Visitor.hpp>

using namespace std;
using namespace Information_Model;

bool operator==(const UA_NodeId& lhs, const UA_NodeId& rhs) {
  return UA_NodeId_equal(&lhs, &rhs);
}

namespace open62541 {
CallbackWrapper::CallbackWrapper(DataType type, ReadCallback read_callback)
    : data_type_(type), readable_(move(read_callback)) {}

CallbackWrapper::CallbackWrapper(DataType type, WriteCallback write_callback)
    : data_type_(type), writable_(move(write_callback)) {}

CallbackWrapper::CallbackWrapper(
    DataType type, ReadCallback read_callback, WriteCallback write_callback)
    : data_type_(type), readable_(move(read_callback)),
      writable_(move(write_callback)) {}

CallbackWrapper::CallbackWrapper(Information_Model::DataType type,
    const ParameterTypes& parameters, ExecuteCallback execute_callback)
    : data_type_(type), parameters_(parameters),
      executable_(move(execute_callback)) {}

CallbackWrapper::CallbackWrapper(Information_Model::DataType type,
    const ParameterTypes& parameters, //
    CallCallback call_callback)
    : data_type_(type), parameters_(parameters),
      callable_(move(call_callback)) {}

void NodeCallbackHandler::initialise(const UA_Logger* logger) {
  logger_ = logger;
  UA_LOG_INFO(logger_, UA_LOGCATEGORY_SERVER, "NodeCallbackHandler initalized");
}

void NodeCallbackHandler::destroy() { callbacks_.clear(); }

CallbackWrapperPtr NodeCallbackHandler::findCallbackWrapper(
    const UA_NodeId* node_id) {
  CallbackWrapperPtr result;
  auto it = callbacks_.find(*node_id);
  if (it != callbacks_.end()) {
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
    UA_LOG_INFO(logger_, UA_LOGCATEGORY_SERVER,
        "Adding callback_wrapper for Node %s", toString(&node_id).c_str());
    callbacks_.emplace(node_id, callback_wrapper);
    status = UA_STATUSCODE_GOOD;
  } else {
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
        "Node %s was already registered earlier", toString(&node_id).c_str());
    status = UA_STATUSCODE_BADNODEIDEXISTS;
  }
  return status;
}

UA_StatusCode NodeCallbackHandler::removeNodeCallbacks(
    const UA_NodeId* node_id) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  auto it = callbacks_.find(*node_id);
  if (it != callbacks_.end()) {
    UA_LOG_INFO(logger_, UA_LOGCATEGORY_SERVER,
        "Removing callbacks for Node %s", toString(node_id).c_str());
    callbacks_.erase(it);
    status = UA_STATUSCODE_GOOD;
  } else {
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
        "Node %s does not have any callback registered",
        toString(node_id).c_str());
    status = UA_STATUSCODE_BADNOTFOUND;
  }
  return status;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
UA_StatusCode NodeCallbackHandler::readNodeValue( // clang-format off
    UA_Server* server,
    [[maybe_unused]] const UA_NodeId* session_id,
    [[maybe_unused]] void* session_context, 
    const UA_NodeId* node_id,
    [[maybe_unused]] void* node_context,
    [[maybe_unused]] UA_Boolean include_source_time_stamp,
    [[maybe_unused]] const UA_NumericRange* range, 
    UA_DataValue* value) { // clang-format on
  UA_StatusCode status = UA_STATUSCODE_BADNOTREADABLE;

  auto it = callbacks_.find(*node_id);
  if (it != callbacks_.end()) {
    UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER,
        "Calling read callback for Node %s", toString(node_id).c_str());
    auto callback_wrapper = it->second;
    try {
      if (auto read = callback_wrapper->readable_) {
        UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER,
            "Calling read callback for Node %s", toString(node_id).c_str());
        auto data_variant = read();
        if (toDataType(data_variant) == callback_wrapper->data_type_) {
          value->value = toUAVariant(data_variant);
          value->hasValue = true;
          status = UA_STATUSCODE_GOOD;
        } else {
          UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
              "Expected to receive %s data type, but received %s instead while "
              "reading Node %s",
              toString(callback_wrapper->data_type_).c_str(),
              toString(toDataType(data_variant)).c_str(),
              toString(node_id).c_str());
          status = UA_STATUSCODE_BADTYPEMISMATCH;
        }
      } else {
        UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
            "Node %s does not have any registered read callback",
            toString(node_id).c_str());
      }
    } catch (const exception& ex) {
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
          "An exception occurred, while reading Node %s value. Exception: %s",
          toString(node_id).c_str(), ex.what());
      removeNodeCallbacks(node_id);
      UA_LOG_WARNING(logger_, UA_LOGCATEGORY_SERVER,
          "Removing Node %s due to an unhandled exception",
          toString(node_id).c_str());
      UA_Server_deleteNode(server, *node_id, true);
      status = UA_STATUSCODE_BADINTERNALERROR;
    }
  } else {
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
        "Node %s does not have any registered read callbacks",
        toString(node_id).c_str());
    status = UA_STATUSCODE_BADNODEIDUNKNOWN;
  }
  return status;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
UA_StatusCode NodeCallbackHandler::writeNodeValue( // clang-format off
    UA_Server* server,
    [[maybe_unused]] const UA_NodeId* session_id,
    [[maybe_unused]] void* session_context, 
    const UA_NodeId* node_id,
    [[maybe_unused]] void* node_context,
    [[maybe_unused]] const UA_NumericRange* range, 
    const UA_DataValue* value) { // clang-format on
  UA_StatusCode status = UA_STATUSCODE_BADNOTWRITABLE;
  auto it = callbacks_.find(*node_id);
  if (it != callbacks_.end()) {
    auto callback_wrapper = it->second;
    if (auto write = callback_wrapper->writable_) {
      UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER,
          "Calling write callback for Node %s", toString(node_id).c_str());
      try {
        auto data_variant = toDataVariant(value->value);
        if (toDataType(data_variant) == callback_wrapper->data_type_) {
          write(data_variant);
          status = UA_STATUSCODE_GOOD;
        } else {
          UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
              "Expected to send %s data type, but sending %s instead while "
              "writing Node %s",
              toString(callback_wrapper->data_type_).c_str(),
              toString(toDataType(data_variant)).c_str(),
              toString(node_id).c_str());
          status = UA_STATUSCODE_BADTYPEMISMATCH;
        }
      } catch (const exception& ex) {
        UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
            "An unhandled exception occurred while trying to write to Node %s "
            "value. Exception: %s",
            toString(node_id).c_str(), ex.what());
        removeNodeCallbacks(node_id);
        UA_LOG_WARNING(logger_, UA_LOGCATEGORY_SERVER,
            "Removing Node %s due to an unhandled exception",
            toString(node_id).c_str());
        UA_Server_deleteNode(server, *node_id, true);
        status = UA_STATUSCODE_BADINTERNALERROR;
      }
    } else {
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
          "Node %s does not have any registered write callback",
          toString(node_id).c_str());
    }
  } else {
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
        "Node %s does not have any registered callbacks",
        toString(node_id).c_str());
    status = UA_STATUSCODE_BADNODEIDUNKNOWN;
  }
  return status;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
UA_StatusCode NodeCallbackHandler::callNodeMethod( // clang-format off
    UA_Server* server,
    [[maybe_unused]] const UA_NodeId* session_id,
    [[maybe_unused]] void* session_context, 
    const UA_NodeId* method_id,
    [[maybe_unused]] void* method_context,
    [[maybe_unused]] const UA_NodeId* object_id, 
    [[maybe_unused]] void* object_context,
    size_t input_size, 
    const UA_Variant* input, 
    size_t output_size,
    UA_Variant* output) { // clang-format on
  UA_StatusCode status = UA_STATUSCODE_BADNOTEXECUTABLE;
  auto it = callbacks_.find(*method_id);
  if (it != callbacks_.end()) {
    auto callback_wrapper = it->second;
    try {
      if (callback_wrapper->parameters_.size() == input_size) {
        Parameters args;
        for (size_t i = 0; i < input_size; ++i) {
          auto data_variant = toDataVariant(input[i]);
          addSupportedParameter(
              args, callback_wrapper->parameters_, i, data_variant);
        }
        if (output_size > 0) {
          if (auto call = callback_wrapper->callable_) {
            UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER,
                "Calling CALL callback for Method %s",
                toString(method_id).c_str());
            auto result_variant = call(args);
            auto ua_variant = toUAVariant(result_variant);
            UA_Variant_copy(&ua_variant, output);
            UA_Variant_clear(&ua_variant);
            status = UA_STATUSCODE_GOOD;
          } else {
            UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
                "Method %s does not have a registered CALL callback",
                toString(method_id).c_str());
          }
        } else {
          if (auto execute = callback_wrapper->executable_) {
            UA_LOG_TRACE(logger_, UA_LOGCATEGORY_SERVER,
                "Calling EXECUTE callback for Method %s",
                toString(method_id).c_str());
            execute(args);
            status = UA_STATUSCODE_GOOD;
          } else {
            UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
                "Method %s does not have a registered EXECUTE callback",
                toString(method_id).c_str());
          }
        }
      } else {
        status = UA_STATUSCODE_BADTOOMANYARGUMENTS;
      }
    } catch (const invalid_argument& ex) {
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
          "Provided Method %s argument is not supported: %s",
          toString(method_id).c_str(), ex.what());
      status = UA_STATUSCODE_BADINVALIDARGUMENT;
    } catch (const range_error& ex) {
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
          "Provided Method %s argument is out of range: %s ",
          toString(method_id).c_str(), ex.what());
      status = UA_STATUSCODE_BADOUTOFRANGE;
    } catch (const exception& ex) {
      UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
          "An unhandled exception occurred while trying to CALL or EXECUTE "
          "%s callback. Exception: %s",
          toString(method_id).c_str(), ex.what());
      removeNodeCallbacks(method_id);
      UA_LOG_WARNING(logger_, UA_LOGCATEGORY_SERVER,
          "Removing Node %s due to an unhandled exception",
          toString(method_id).c_str());
      UA_Server_deleteNode(server, *method_id, true);
      status = UA_STATUSCODE_BADNOCOMMUNICATION;
    }
  } else {
    UA_LOG_ERROR(logger_, UA_LOGCATEGORY_SERVER,
        "Method %s does not have any registered callbacks",
        toString(method_id).c_str());
    status = UA_STATUSCODE_BADNODEIDUNKNOWN;
  }
  return status;
}

NodeCallbackHandler::CallbackMap
    NodeCallbackHandler::callbacks_; // NOLINT(cert-err58-cpp)
const UA_Logger* NodeCallbackHandler::logger_; // NOLINT
} // namespace open62541
