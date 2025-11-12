#include "CallbackRepo.hpp"
#include "StringConverter.hpp"
#include "VariantConverter.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <open62541/plugin/log.h>
#include <stdexcept>

namespace open62541 {
using namespace std;
using namespace HaSLL;
using namespace Information_Model;

struct NoCallbackRepo : runtime_error {
  NoCallbackRepo() : runtime_error("") {}
};

struct CallbackNotFound : runtime_error {
  CallbackNotFound() : runtime_error("") {}
};

struct BadOperation : runtime_error {
  explicit BadOperation(const string& msg) : runtime_error(msg) {}
};

struct ServerNotSet : runtime_error {
  ServerNotSet() : runtime_error("") {}
};

struct NotReadable : runtime_error {
  NotReadable() : runtime_error("") {}
};

struct NotWritable : runtime_error {
  NotWritable() : runtime_error("") {}
};

struct NotCallable : runtime_error {
  NotCallable() : runtime_error("") {}
};

CallbackRepo* getCallbackRepo(void* context_ptr) {
  if (context_ptr == nullptr) {
    throw NoCallbackRepo();
  }

  auto* repo = static_cast<CallbackRepo*>(context_ptr);
  if (repo == nullptr) {
    throw NoCallbackRepo();
  }
  return repo;
}

UA_Logger* getLogger(UA_Server* server) {
  auto* config = UA_Server_getConfig(server);
  if (config == nullptr) {
    throw ServerNotSet();
  }
  if (config->logging == nullptr) {
    // treat it the same, since logging is always set for a valid server
    throw ServerNotSet();
  }
  return config->logging;
}

// @kudos to Jon Kalb and Lisa Lippincott for this technique
// @see more information on
// https://cppsecrets.blogspot.com/2013/12/using-lippincott-function-for.html
UA_StatusCode handleExceptions(
    UA_Server* server, const UA_NodeId* node_id) noexcept {
  try {
    throw; // rethrow current exception
  } catch (const BadOperation& ex) {
    UA_LOG_WARNING(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Removing Node %s due to exception %s", toString(node_id).c_str(),
        ex.what());
    UA_Server_deleteNode(server, *node_id, true);
    return UA_STATUSCODE_BADINTERNALERROR;
  } catch (const CallbackNotFound&) {
    UA_LOG_ERROR(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Node %s does not have any registered callbacks",
        toString(node_id).c_str());
    return UA_STATUSCODE_BADNODEIDUNKNOWN;
  } catch (const NoCallbackRepo&) {
    UA_LOG_FATAL(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Node %s context has no access to callback repository",
        toString(node_id).c_str());
    return UA_STATUSCODE_BADINTERNALERROR;
  } catch (const ServerNotSet&) {
    return UA_STATUSCODE_BADINTERNALERROR;
  } catch (...) {
    UA_LOG_FATAL(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Caught an unhandled error while trying to  value from Node %s",
        toString(node_id).c_str());
    return UA_STATUSCODE_BADUNEXPECTEDERROR;
  }
}

UA_StatusCode readNodeValue(UA_Server* server, const UA_NodeId*, void*,
    const UA_NodeId* node_id, void* node_context, UA_Boolean,
    const UA_NumericRange*, UA_DataValue* value) {
  try {
    UA_LOG_TRACE(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Calling read callback for Node %s", toString(node_id).c_str());
    auto* repo = getCallbackRepo(node_context);
    return repo->read(node_id, value);
  } catch (const NotReadable&) {
    UA_LOG_ERROR(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Node %s is not readable", toString(node_id).c_str());
    return UA_STATUSCODE_BADNOTREADABLE;
  } catch (...) {
    return handleExceptions(server, node_id);
  }
}

UA_StatusCode writeNodeValue(UA_Server* server, const UA_NodeId*, void*,
    const UA_NodeId* node_id, void* node_context, const UA_NumericRange*,
    const UA_DataValue* value) {
  try {
    auto* repo = getCallbackRepo(node_context);
    return repo->write(node_id, value);
  } catch (const NotWritable&) {
    UA_LOG_ERROR(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Node %s is not writable", toString(node_id).c_str());
    return UA_STATUSCODE_BADNOTWRITABLE;
  } catch (...) {
    return handleExceptions(server, node_id);
  }
}

UA_StatusCode callNodeMethod(UA_Server* server, const UA_NodeId*, void*,
    const UA_NodeId* method_id, void* method_context, const UA_NodeId*, void*,
    size_t input_size, const UA_Variant* input, size_t output_size,
    UA_Variant* output) {
  try {
    auto* repo = getCallbackRepo(method_context);
    return repo->execute(method_id, input_size, input, output_size, output);
  } catch (const NotWritable&) {
    UA_LOG_ERROR(getLogger(server), UA_LOGCATEGORY_SERVER,
        "Node %s is not executable", toString(method_id).c_str());
    return UA_STATUSCODE_BADNOTEXECUTABLE;
  } catch (...) {
    return handleExceptions(server, method_id);
  }
}

CallbackRepo::CallbackRepo()
    : logger_(LoggerManager::registerLogger("Open62541::CallbackRepo")) {}

UA_StatusCode CallbackRepo::add(
    UA_NodeId node_id, const CallbackWrapper& wrapper) {
  if (std::holds_alternative<monostate>(wrapper)) {
    throw CallbackNotFound();
  }

  UA_StatusCode status = UA_STATUSCODE_GOOD;
  callbacks_.visit(node_id,
      [&status](const auto&) { status = UA_STATUSCODE_BADNODEIDEXISTS; });
  if (status != UA_STATUSCODE_GOOD) {
    logger_->error(
        "Node {} was already registered earlier", toString(&node_id));
  } else {
    logger_->trace("Adding wrapper for Node {}", toString(&node_id));
    callbacks_.emplace(node_id, wrapper);
  }
  return status;
}

void CallbackRepo::remove(const UA_NodeId* node_id) {
  logger_->trace("Removing callbacks for Node {}", toString(node_id));
  callbacks_.erase_if(
      [node_id](const auto& pair) { return pair.first == *node_id; });
}

CallbackWrapper CallbackRepo::find(const UA_NodeId* node_id) {
  CallbackWrapper result;
  callbacks_.visit(
      *node_id, [&result](const auto& pair) { result = pair.second; });
  if (std::holds_alternative<monostate>(result)) {
    throw CallbackNotFound();
  }
  return result;
}

UA_StatusCode CallbackRepo::read(
    const UA_NodeId* node_id, UA_DataValue* value) {
  logger_->trace("Calling read callback for Node {}", toString(node_id));

  try {
    DataType target_type = DataType::Unknown;
    DataVariant result;

    auto wrapper = find(node_id);
    if (std::holds_alternative<ObservablePtr>(wrapper)) {
      auto observable = std::get<ObservablePtr>(wrapper);
      logger_->trace("Calling read from Observable Node {}", toString(node_id));
      target_type = observable->dataType();
      result = observable->read();
    } else if (std::holds_alternative<WritablePtr>(wrapper)) {
      auto writable = std::get<WritablePtr>(wrapper);
      logger_->trace("Calling read from Writable Node {}", toString(node_id));
      target_type = writable->dataType();
      if (writable->isWriteOnly()) {
        // set default data as dummy to avoid bad internal error
        // writable can not have None or Unknown data type
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        result = setVariant(target_type).value();
        logger_->warning(
            "Node {} does not support read operation", toString(node_id));
      } else {
        result = writable->read();
      }
    } else {
      auto readable = std::get<ReadablePtr>(wrapper);
      logger_->trace("Calling read from Readable Node {}", toString(node_id));
      target_type = readable->dataType();
      result = readable->read();
    }

    if (toDataType(result) == target_type) {
      value->value = toUAVariant(result);
      value->hasValue = true;
      return UA_STATUSCODE_GOOD;
    } else {
      logger_->error("Expected to receive {} data type, but received {} "
                     "instead from Node {}",
          toString(target_type), toString(toDataType(result)),
          toString(node_id));
      return UA_STATUSCODE_BADTYPEMISMATCH;
    }
  } catch (const bad_variant_access&) {
    throw NotReadable();
  } catch (const CallbackNotFound&) {
    throw; // rethrow CallbackNotFound
  } catch (const exception& ex) {
    remove(node_id);
    throw BadOperation(ex.what());
  }
}

UA_StatusCode CallbackRepo::write(
    const UA_NodeId* node_id, const UA_DataValue* value) {
  logger_->trace("Calling write callback for Node %s", toString(node_id));
  try {
    auto writable = std::get<WritablePtr>(find(node_id));
    auto data_variant = toDataVariant(value->value);
    if (toDataType(data_variant) == writable->dataType()) {
      writable->write(data_variant);
      return UA_STATUSCODE_GOOD;
    } else {
      logger_->error("Expected to write {} data type, but writing {} instead "
                     "for Node {}",
          toString(writable->dataType()), toString(toDataType(data_variant)),
          toString(node_id));
      return UA_STATUSCODE_BADTYPEMISMATCH;
    }
  } catch (const bad_variant_access&) {
    throw NotWritable();
  } catch (const CallbackNotFound&) {
    throw; // rethrow CallbackNotFound
  } catch (const exception& ex) {
    remove(node_id);
    throw BadOperation(ex.what());
  }
}

UA_StatusCode CallbackRepo::execute(const UA_NodeId* method_id,
    size_t input_size, const UA_Variant* input, size_t output_size,
    UA_Variant* output) {
  logger_->trace("Calling Method callback for Node {}", toString(method_id));
  try {
    auto callable = std::get<CallablePtr>(find(method_id));

    auto supported_params = callable->parameterTypes();
    if (supported_params.size() < input_size) {
      logger_->error(
          "Too many arguments provided for Method {}", toString(method_id));
      return UA_STATUSCODE_BADTOOMANYARGUMENTS;
    }

    Parameters params;
    for (size_t i = 0; i < input_size; ++i) {
      auto parameter = toDataVariant(input[i]);
      addSupportedParameter(params, supported_params, i, parameter);
    }

    if (output_size == 0) {
      logger_->trace(
          "Calling Method callback for Node {}", toString(method_id));
      callable->execute(params);
      return UA_STATUSCODE_GOOD;
    }

    logger_->trace("Calling Method callback with results for Node {}",
        toString(method_id));
    auto result_variant = callable->call(params);
    if (toDataType(result_variant) == callable->resultType()) {
      auto ua_variant = toUAVariant(result_variant);
      UA_Variant_copy(&ua_variant, output);
      UA_Variant_clear(&ua_variant);
      return UA_STATUSCODE_GOOD;
    } else {
      logger_->error("Expected to receive {} data type, but received {} "
                     "instead from Method {}",
          toString(callable->resultType()),
          toString(toDataType(result_variant)), toString(method_id));
      return UA_STATUSCODE_BADTYPEMISMATCH;
    }
  } catch (const ParameterDoesNotExist& ex) {
    logger_->error("Method {} {}", toString(method_id), ex.what());
    return UA_STATUSCODE_BADINVALIDARGUMENT;
  } catch (const ParameterTypeMismatch& ex) {
    logger_->error("Method {} {}", toString(method_id), ex.what());
    return UA_STATUSCODE_BADINVALIDARGUMENT;
  } catch (const MandatoryParameterHasNoValue& ex) {
    logger_->error("Method {} {}", toString(method_id), ex.what());
    return UA_STATUSCODE_BADARGUMENTSMISSING;
  } catch (const bad_variant_access&) {
    throw NotCallable();
  } catch (const CallbackNotFound&) {
    throw; // rethrow CallbackNotFound
  } catch (const exception& ex) {
    remove(method_id);
    throw BadOperation(ex.what());
  }
}
} // namespace open62541
