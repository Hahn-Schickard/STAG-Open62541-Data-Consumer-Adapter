#ifndef __OPCUA_NULL_LOGGER_HPP
#define __OPCUA_NULL_LOGGER_HPP

#include <open62541/plugin/log.h>

extern "C" {
extern const UA_Logger Null_Logger_;
extern const UA_Logger *Null_Logger;

void Null_Logger_log(void *_, UA_LogLevel level, UA_LogCategory category,
                     const char *msg, va_list args);

void Null_Logger_clear(void *logContext);
}

#endif //__OPCUA_NULL_LOGGER_HPP