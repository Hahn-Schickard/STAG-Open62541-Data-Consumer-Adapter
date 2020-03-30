#ifndef __OPCUA_NULL_LOGGER_HPP
#define __OPCUA_NULL_LOGGER_HPP

#include <open62541/plugin/log.h>

#include "Logger.hpp"

extern "C" {
extern const UA_Logger HaSLL_Logger_;

void registerLoggers();
void removeLoggers();

void HaSLL_Logger_log(void *_, UA_LogLevel level, UA_LogCategory category,
                      const char *msg, va_list args);

void HaSLL_Logger_clear(void *logContext);
}

#endif //__OPCUA_NULL_LOGGER_HPP