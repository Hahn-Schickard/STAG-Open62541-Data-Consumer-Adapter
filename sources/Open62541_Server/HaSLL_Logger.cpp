#include "HaSLL_Logger.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

using namespace std;
using namespace HaSLI;

LoggerPtr network_logger;
LoggerPtr channel_logger;
LoggerPtr session_logger;
LoggerPtr server_logger;
LoggerPtr client_logger;
LoggerPtr userland_logger;
LoggerPtr security_policy_logger;

void registerLoggers() {
  network_logger = LoggerManager::registerLogger("Open62541 Network Layer");
  channel_logger = LoggerManager::registerLogger("Open62541 Channel Layer");
  session_logger = LoggerManager::registerLogger("Open62541 Session Layer");
  server_logger = LoggerManager::registerLogger("Open62541 Server Layer");
  client_logger = LoggerManager::registerLogger("Open62541 Client Layer");
  userland_logger = LoggerManager::registerLogger("Open62541 User Layer");
  security_policy_logger =
      LoggerManager::registerLogger("Open62541 Security Layer");
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
  network_logger->deregister();
  channel_logger->deregister();
  session_logger->deregister();
  server_logger->deregister();
  client_logger->deregister();
  userland_logger->deregister();
  security_policy_logger->deregister();

  network_logger.reset();
  channel_logger.reset();
  session_logger.reset();
  server_logger.reset();
  client_logger.reset();
  userland_logger.reset();
  security_policy_logger.reset();
}

void HaSLL_Logger_log(UNUSED(void* _logContext), UA_LogLevel level,
    UA_LogCategory category, const char* msg, va_list args) {

  size_t buffer_size = strlen(msg);
  char* buffer = (char*)(malloc(sizeof(char) * buffer_size));
  vsnprintf(buffer, buffer_size + 1, msg, args);
  string message = string(buffer);

  switch (category) {
  case UA_LogCategory::UA_LOGCATEGORY_NETWORK: {
    if (network_logger) {
      network_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURECHANNEL: {
    if (channel_logger) {
      channel_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SESSION: {
    if (session_logger) {
      session_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SERVER: {
    if (server_logger) {
      server_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_CLIENT: {
    if (server_logger) {
      client_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_USERLAND: {
    if (userland_logger) {
      userland_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  case UA_LogCategory::UA_LOGCATEGORY_SECURITYPOLICY: {
    if (security_policy_logger) {
      security_policy_logger->log(getLoggingLevel(level), message);
    }
    break;
  }
  }
}

void HaSLL_Logger_clear(UNUSED(void* logContext)) {
  // Nothing to clear
}

const UA_Logger HaSLL_Logger_ = {HaSLL_Logger_log, NULL, HaSLL_Logger_clear};
const UA_Logger* HaSLL_Logger = &HaSLL_Logger_;