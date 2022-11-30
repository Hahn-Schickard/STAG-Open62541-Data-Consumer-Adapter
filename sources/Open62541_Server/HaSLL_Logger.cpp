#include "HaSLL_Logger.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

#include <map>

using namespace std;
using namespace HaSLI;

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

void registerLoggers() {
  if (loggers.empty()) {
    loggers.emplace(Open62541_Logger::NETWORK,
        LoggerManager::registerLogger("Open62541 Network Layer"));
    loggers.emplace(Open62541_Logger::CHANNEL,
        LoggerManager::registerLogger("Open62541 Channel Layer"));
    loggers.emplace(Open62541_Logger::SESSION,
        LoggerManager::registerLogger("Open62541 Session Layer"));
    loggers.emplace(Open62541_Logger::SERVER,
        LoggerManager::registerLogger("Open62541 Server Layer"));
    loggers.emplace(Open62541_Logger::CLIENT,
        LoggerManager::registerLogger("Open62541 Client Layer"));
    loggers.emplace(Open62541_Logger::USER,
        LoggerManager::registerLogger("Open62541 User Layer"));
    loggers.emplace(Open62541_Logger::SECURITY,
        LoggerManager::registerLogger("Open62541 Security Layer"));
  }
}

SeverityLevel getLoggingLevel(UA_LogLevel level) {
  switch (level) {
  case UA_LogLevel::UA_LOGLEVEL_DEBUG: {
    return SeverityLevel::DEBUG;
  }
  case UA_LogLevel::UA_LOGLEVEL_ERROR: {
    return SeverityLevel::ERROR;
  }
  case UA_LogLevel::UA_LOGLEVEL_FATAL: {
    return SeverityLevel::CRITICAL;
  }
  case UA_LogLevel::UA_LOGLEVEL_INFO: {
    return SeverityLevel::INFO;
  }
  case UA_LogLevel::UA_LOGLEVEL_TRACE: {
    return SeverityLevel::TRACE;
  }
  case UA_LogLevel::UA_LOGLEVEL_WARNING: {
    return SeverityLevel::WARNING;
  }
  default: {
    return SeverityLevel::ERROR;
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

void HaSLL_Logger_log(UNUSED(void* _logContext), UA_LogLevel level,
    UA_LogCategory category, const char* msg, va_list args) {

  size_t buffer_size = strlen(msg);
  auto* buffer = (char*)(malloc(sizeof(char) * buffer_size));
  vsnprintf(buffer, buffer_size + 1, msg, args);
  auto message = string(buffer);

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

void HaSLL_Logger_clear(UNUSED(void* logContext)) {
  // Nothing to clear
}

const UA_Logger HaSLL_Logger_ = {HaSLL_Logger_log, nullptr, HaSLL_Logger_clear};
const UA_Logger* HaSLL_Logger = &HaSLL_Logger_;