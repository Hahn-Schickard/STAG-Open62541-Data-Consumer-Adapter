#include "Logger.hpp"
#include "HaSLL/LoggerManager.hpp"

#include <unordered_map>

namespace open62541 {
using namespace std;
using namespace HaSLL;

using LoggersMap = unordered_map<UA_LogCategory, LoggerPtr>;

LoggersMap* registerLoggers() {
  auto* result = new LoggersMap;
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_NETWORK,
      LoggerManager::registerLogger("Open62541::Network"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL,
      LoggerManager::registerLogger("Open62541::Channel"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_SESSION,
      LoggerManager::registerLogger("Open62541::Session"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_SERVER,
      LoggerManager::registerLogger("Open62541::Server"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_CLIENT,
      LoggerManager::registerLogger("Open62541::Client"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_USERLAND,
      LoggerManager::registerLogger("Open62541::User"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY,
      LoggerManager::registerLogger("Open62541::Security"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_EVENTLOOP,
      LoggerManager::registerLogger("Open62541::EventLoop"));
  result->emplace(UA_LogCategory::UA_LOGCATEGORY_DISCOVERY,
      LoggerManager::registerLogger("Open62541::Discovery"));
  return result;
}

SeverityLevel getLoggingLevel(UA_LogLevel level) {
  switch (level) {
  case UA_LogLevel::UA_LOGLEVEL_DEBUG: {
    return SeverityLevel::Debug;
  }
  case UA_LogLevel::UA_LOGLEVEL_ERROR: {
    return SeverityLevel::Error;
  }
  case UA_LogLevel::UA_LOGLEVEL_FATAL: {
    return SeverityLevel::Critical;
  }
  case UA_LogLevel::UA_LOGLEVEL_INFO: {
    return SeverityLevel::Info;
  }
  case UA_LogLevel::UA_LOGLEVEL_TRACE: {
    return SeverityLevel::Trace;
  }
  case UA_LogLevel::UA_LOGLEVEL_WARNING: {
    return SeverityLevel::Warning;
  }
  default: {
    return SeverityLevel::Error;
  }
  }
}

LoggersMap* getLoggers(void* context_ptr) {
  return static_cast<LoggersMap*>(context_ptr);
}

void logToHaSLL(void* context, UA_LogLevel level, UA_LogCategory category,
    const char* msg, va_list args) {
  /*
   * Kudos to Chris Dodd @stackoverflow
   * https://stackoverflow.com/a/70749935
   */
  std::string message;
  va_list args_copy;
  va_copy(args_copy, args); // make a copy for the buffer size calculation
  // NOLINTNEXTLINE(clang-analyzer-valist.*)
  auto len = vsnprintf(
      nullptr, 0, msg, args_copy); // get the amount of bytes needed to write
  // vsnprintf returns a negative value if an error occurred
  if (len > 0) {
    auto padded_len = static_cast<size_t>(len) + 1;
    message.resize(padded_len); // Add space for NULL terminator
    // clang-tidy bug for vsnprintf() @see
    // https://bugs.llvm.org/show_bug.cgi?id=41311
    // NOLINTNEXTLINE(clang-analyzer-valist.*, cert-err33-c, readability-*)
    vsnprintf(
        message.data(), padded_len, msg, args); // write args into the message
    message.resize(static_cast<size_t>(len)); // Remove the NULL terminator
  } // else -> message will be empty

  if (auto* loggers = getLoggers(context); loggers != nullptr) {
    if (auto it = loggers->find(category); it != loggers->end()) {
      it->second->log(getLoggingLevel(level), message);
    }
  }
}

void destroyHaSLL(struct UA_Logger* logger) {
  if (logger != nullptr) {
    if (auto* loggers = getLoggers(logger->context); loggers != nullptr) {
      loggers->clear();
      delete loggers;
    }
    UA_free(logger);
  }
}
} // namespace open62541

UA_Logger* createHaSLL() {
  auto* loggers = open62541::registerLoggers();
  auto* logger = (UA_Logger*)UA_malloc(sizeof(UA_Logger));
  if (logger == nullptr) {
    return nullptr;
  }
  *logger = {open62541::logToHaSLL, loggers, open62541::destroyHaSLL};
  return logger;
}