#include "Logger.hpp"
#include "HaSLL/LoggerManager.hpp"

#include <map>
#include <mutex>

namespace open62541 {
using namespace std;
using namespace HaSLL;

enum class LoggerType : uint8_t {
  Network,
  Channel,
  Session,
  Server,
  Client,
  User,
  Security,
  EventLoop,
  Discovery
};

map<LoggerType, LoggerPtr> loggers;
std::mutex logger_mutex;

void registerLoggers() {
  if (loggers.empty()) {
    loggers.emplace(LoggerType::Network,
        LoggerManager::registerLogger("Open62541::Network"));
    loggers.emplace(LoggerType::Channel,
        LoggerManager::registerLogger("Open62541::Channel"));
    loggers.emplace(LoggerType::Session,
        LoggerManager::registerLogger("Open62541::Session"));
    loggers.emplace(
        LoggerType::Server, LoggerManager::registerLogger("Open62541::Server"));
    loggers.emplace(
        LoggerType::Client, LoggerManager::registerLogger("Open62541::Client"));
    loggers.emplace(
        LoggerType::User, LoggerManager::registerLogger("Open62541::User"));
    loggers.emplace(LoggerType::Security,
        LoggerManager::registerLogger("Open62541::Security"));
    loggers.emplace(LoggerType::EventLoop,
        LoggerManager::registerLogger("Open62541::EventLoop"));
    loggers.emplace(LoggerType::Discovery,
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
  const std::lock_guard<std::mutex> lock(logger_mutex);

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

  switch (category) {
  case UA_LogCategory::UA_LOGCATEGORY_NETWORK: {
    auto it = loggers.find(LoggerType::Network);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL: {
    auto it = loggers.find(LoggerType::Channel);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SESSION: {
    auto it = loggers.find(LoggerType::Session);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SERVER: {
    auto it = loggers.find(LoggerType::Server);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_CLIENT: {
    auto it = loggers.find(LoggerType::Client);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_USERLAND: {
    auto it = loggers.find(LoggerType::User);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY: {
    auto it = loggers.find(LoggerType::Security);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_EVENTLOOP: {
    auto it = loggers.find(LoggerType::EventLoop);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_DISCOVERY: {
    auto it = loggers.find(LoggerType::Discovery);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_PUBSUB: {
    auto it = loggers.find(LoggerType::Server);
    if (it != loggers.end()) {
      it->second->error("PubSub is not supported by the server");
    }
    break;
  }
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