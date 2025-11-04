#include "HaSLL_Logger.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

#include <map>
#include <mutex>

using namespace std;
using namespace HaSLL;

// NOLINTNEXTLINE(readability-identifier-naming)
enum class Open62541_Logger : uint8_t {
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

map<Open62541_Logger, LoggerPtr> loggers;
std::mutex logger_mutex;

void registerLoggers() {
  if (loggers.empty()) {
    loggers.emplace(Open62541_Logger::Network,
        LoggerManager::registerLogger("Open62541::Network"));
    loggers.emplace(Open62541_Logger::Channel,
        LoggerManager::registerLogger("Open62541::Channel"));
    loggers.emplace(Open62541_Logger::Session,
        LoggerManager::registerLogger("Open62541::Session"));
    loggers.emplace(Open62541_Logger::Server,
        LoggerManager::registerLogger("Open62541::Server"));
    loggers.emplace(Open62541_Logger::Client,
        LoggerManager::registerLogger("Open62541::Client"));
    loggers.emplace(Open62541_Logger::User,
        LoggerManager::registerLogger("Open62541::User"));
    loggers.emplace(Open62541_Logger::Security,
        LoggerManager::registerLogger("Open62541::Security"));
    loggers.emplace(Open62541_Logger::EventLoop,
        LoggerManager::registerLogger("Open62541::EventLoop"));
    loggers.emplace(Open62541_Logger::Discovery,
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
      0, 0, msg, args_copy); // get the amount of bytes needed to write
  // vsnprintf returns a negative value if an error occurred
  if (len > 0) {
    message.resize(len + 1); // Add space for NULL terminator
    // clang-tidy bug for vsnprintf() @see
    // https://bugs.llvm.org/show_bug.cgi?id=41311
    // NOLINTNEXTLINE(clang-analyzer-valist.*, cert-err33-c, readability-*)
    vsnprintf(&message[0], len + 1, msg, args); // write args into the message
    message.resize(len); // Remove the NULL terminator
  } // else -> message will be empty

  switch (category) {
  case UA_LogCategory::UA_LOGCATEGORY_NETWORK: {
    auto it = loggers.find(Open62541_Logger::Network);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL: {
    auto it = loggers.find(Open62541_Logger::Channel);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SESSION: {
    auto it = loggers.find(Open62541_Logger::Session);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SERVER: {
    auto it = loggers.find(Open62541_Logger::Server);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_CLIENT: {
    auto it = loggers.find(Open62541_Logger::Client);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_USERLAND: {
    auto it = loggers.find(Open62541_Logger::User);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY: {
    auto it = loggers.find(Open62541_Logger::Security);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_EVENTLOOP: {
    auto it = loggers.find(Open62541_Logger::EventLoop);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_DISCOVERY: {
    auto it = loggers.find(Open62541_Logger::Discovery);
    if (it != loggers.end()) {
      it->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_PUBSUB: {
    auto it = loggers.find(Open62541_Logger::Server);
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

UA_Logger* createHaSLL() {
  auto* logger = (UA_Logger*)UA_malloc(sizeof(UA_Logger));
  if (logger == nullptr) {
    return nullptr;
  }
  *logger = {logToHaSLL, nullptr, destroyHaSLL};
  return logger;
}