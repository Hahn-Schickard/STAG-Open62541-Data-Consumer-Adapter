#include "HaSLL_Logger.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

#include <map>
#include <mutex>

using namespace std;
using namespace HaSLL;

// NOLINTNEXTLINE(readability-identifier-naming)
enum class Open62541_Logger {
  NETWORK,
  CHANNEL,
  SESSION,
  SERVER,
  CLIENT,
  USER,
  SECURITY
};

map<Open62541_Logger, LoggerPtr> loggers;
std::mutex logger_mutex;

void registerLoggers() {
  if (loggers.empty()) {
    loggers.emplace(Open62541_Logger::NETWORK,
        LoggerManager::registerLogger("Open62541::Network"));
    loggers.emplace(Open62541_Logger::CHANNEL,
        LoggerManager::registerLogger("Open62541::Channel"));
    loggers.emplace(Open62541_Logger::SESSION,
        LoggerManager::registerLogger("Open62541::Session"));
    loggers.emplace(Open62541_Logger::SERVER,
        LoggerManager::registerLogger("Open62541::Server"));
    loggers.emplace(Open62541_Logger::CLIENT,
        LoggerManager::registerLogger("Open62541::Client"));
    loggers.emplace(Open62541_Logger::USER,
        LoggerManager::registerLogger("Open62541::User"));
    loggers.emplace(Open62541_Logger::SECURITY,
        LoggerManager::registerLogger("Open62541::Security"));
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
  } catch (...) {
    // surpress any exceptions thrown from logger dtors
  }
}

// NOLINTNEXTLINE(readability-identifier-naming)
void HaSLL_Logger_log([[maybe_unused]] void* _logContext, UA_LogLevel level,
    UA_LogCategory category, const char* msg, va_list args) {
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
      // NOLINTNEXTLINE(modernize-use-nullptr)
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
    auto network_logger = loggers.find(Open62541_Logger::NETWORK);
    if (network_logger != loggers.end()) {
      network_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL: {
    auto channel_logger = loggers.find(Open62541_Logger::CHANNEL);
    if (channel_logger != loggers.end()) {
      channel_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SESSION: {
    auto session_logger = loggers.find(Open62541_Logger::SESSION);
    if (session_logger != loggers.end()) {
      session_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SERVER: {
    auto server_logger = loggers.find(Open62541_Logger::SERVER);
    if (server_logger != loggers.end()) {
      server_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_CLIENT: {
    auto client_logger = loggers.find(Open62541_Logger::CLIENT);
    if (client_logger != loggers.end()) {
      client_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_USERLAND: {
    auto userland_logger = loggers.find(Open62541_Logger::USER);
    if (userland_logger != loggers.end()) {
      userland_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY: {
    auto security_policy_logger = loggers.find(Open62541_Logger::SECURITY);
    if (security_policy_logger != loggers.end()) {
      security_policy_logger->second->log(getLoggingLevel(level), message);
    }
    break;
  }
  }
}

void HaSLL_Logger_clear([[maybe_unused]] void* log_context) {
  // Nothing to clear
}

// NOLINTNEXTLINE
const UA_Logger HaSLL_Logger_ = {HaSLL_Logger_log, NULL, HaSLL_Logger_clear};
const UA_Logger* HaSLL_Logger = &HaSLL_Logger_; // NOLINT