#include "Logger.hpp"
#include "HaSLL/LoggerManager.hpp"

#include <map>
#include <mutex>

namespace open62541 {
using namespace std;
using namespace HaSLL;

map<UA_LogCategory, LoggerPtr> loggers;
std::mutex logger_mutex;

void registerLoggers() {
  if (loggers.empty()) {
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_NETWORK,
        LoggerManager::registerLogger("Open62541::Network"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL,
        LoggerManager::registerLogger("Open62541::Channel"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_SESSION,
        LoggerManager::registerLogger("Open62541::Session"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_SERVER,
        LoggerManager::registerLogger("Open62541::Server"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_CLIENT,
        LoggerManager::registerLogger("Open62541::Client"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_USERLAND,
        LoggerManager::registerLogger("Open62541::User"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY,
        LoggerManager::registerLogger("Open62541::Security"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_EVENTLOOP,
        LoggerManager::registerLogger("Open62541::EventLoop"));
    loggers.emplace(UA_LogCategory::UA_LOGCATEGORY_DISCOVERY,
        LoggerManager::registerLogger("Open62541::Discovery"));
  }
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

void removeLoggers() {
  try {
    loggers.clear();
  } catch (...) { // NOLINT(bugprone-empty-catch)
    // surpress any exceptions thrown from logger dtors
  }
}

void logToHaSLL(void*, UA_LogLevel level, UA_LogCategory category,
    const char* msg, va_list args) {
  lock_guard lock(logger_mutex);

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

  if (auto it = loggers.find(category); it != loggers.end()) {
    it->second->log(getLoggingLevel(level), message);
  }
}

void destroyHaSLL(struct UA_Logger* logger) {
  removeLoggers();
  UA_free(logger);
}
} // namespace open62541

UA_Logger* createHaSLL() {
  open62541::registerLoggers();
  auto* logger = (UA_Logger*)UA_malloc(sizeof(UA_Logger));
  if (logger == nullptr) {
    return nullptr;
  }
  *logger = {open62541::logToHaSLL, nullptr, open62541::destroyHaSLL};
  return logger;
}