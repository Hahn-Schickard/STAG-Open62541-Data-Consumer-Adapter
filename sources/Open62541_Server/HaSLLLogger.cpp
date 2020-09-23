#include "HaSLLLogger.hpp"
#include "LoggerRepository.hpp"

using namespace std;
using namespace HaSLL;

shared_ptr<HaSLL::Logger> network_logger;
shared_ptr<HaSLL::Logger> channel_logger;
shared_ptr<HaSLL::Logger> session_logger;
shared_ptr<HaSLL::Logger> server_logger;
shared_ptr<HaSLL::Logger> client_logger;
shared_ptr<HaSLL::Logger> userland_logger;
shared_ptr<HaSLL::Logger> security_policy_logger;

void registerLoggers() {
  network_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 Network Layer");
  channel_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 Channel Layer");
  session_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 Session Layer");
  server_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 Server Layer");
  client_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 Client Layer");
  userland_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 User Layer");
  security_policy_logger =
      LoggerRepository::getInstance().registerLoger("Open62541 Security Layer");
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
    return SeverityLevel::WARNNING;
  }
  default: { return SeverityLevel::ERROR; }
  }
}

void removeLoggers() {
  LoggerRepository::getInstance().deregisterLoger("Open62541 Network Layer");
  LoggerRepository::getInstance().deregisterLoger("Open62541 Channel Layer");
  LoggerRepository::getInstance().deregisterLoger("Open62541 Session Layer");
  LoggerRepository::getInstance().deregisterLoger("Open62541 Server Layer");
  LoggerRepository::getInstance().deregisterLoger("Open62541 Client Layer");
  LoggerRepository::getInstance().deregisterLoger("Open62541 User Layer");
  LoggerRepository::getInstance().deregisterLoger("Open62541 Security Layer");

  network_logger.reset();
  channel_logger.reset();
  session_logger.reset();
  server_logger.reset();
  client_logger.reset();
  userland_logger.reset();
  security_policy_logger.reset();
}

void HaSLL_Logger_log(void *_logContext, UA_LogLevel level,
                      UA_LogCategory category, const char *msg, va_list args) {

  size_t buffer_size = strlen(msg);
  char *buffer = (char *)(malloc(sizeof(char) * buffer_size));
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

void HaSLL_Logger_clear(void *logContext) {
  // Nothing to clear
}

const UA_Logger HaSLL_Logger_ = {HaSLL_Logger_log, NULL, HaSLL_Logger_clear};
const UA_Logger *HaSLL_Logger = &HaSLL_Logger_;