#ifndef __OPEN62541_LOGGER_HPP
#define __OPEN62541_LOGGER_HPP

#include <open62541/plugin/log.h>

extern "C" {
UA_Logger* createHaSLL();
}

#endif //__OPEN62541_NULL_LOGGER_HPP