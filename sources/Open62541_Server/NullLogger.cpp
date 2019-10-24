#include "NullLogger.hpp"

void Null_Logger_log(void *_logContext, UA_LogLevel level,
                     UA_LogCategory category, const char *msg, va_list args) {
  // Log nothing
}

void Null_Logger_clear(void *logContext) {
  // Nothing to clear
}

const UA_Logger Null_Logger_ = {Null_Logger_log, NULL, Null_Logger_clear};
const UA_Logger *Null_Logger = &Null_Logger_;