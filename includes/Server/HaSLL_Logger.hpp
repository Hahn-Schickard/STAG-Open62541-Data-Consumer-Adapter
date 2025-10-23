#ifndef __OPCUA_NULL_LOGGER_HPP
#define __OPCUA_NULL_LOGGER_HPP

#include <open62541/plugin/log.h>

extern "C" {
UA_Logger* createHaSLL();
}

#endif //__OPCUA_NULL_LOGGER_HPP