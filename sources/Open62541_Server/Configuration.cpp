#include "Configuration.hpp"
#include "HaSLLLogger.hpp"

#include <open62541/server_config_default.h>

using namespace std;

namespace open62541 {
Configuration::Configuration() {
  try {
    if ((configuration_ =
             (UA_ServerConfig *)calloc(1, sizeof(UA_ServerConfig)))) {
      configuration_->logger.log = HaSLL_Logger_.log;
      configuration_->logger.clear = HaSLL_Logger_.clear;
      // @TODO: Implement Config.hpp and Config_Serializer.hpp usage to set
      // UA_Config
      UA_ServerConfig_setDefault(configuration_);
    } else {
      throw Open62541_Config_Exception(
          "Failed to allocate Open62541 configuration");
    }
  } catch (exception &ex) {
    string error_msg =
        "Caught exception when deserializing Configuration file: " +
        string(ex.what());
    throw Open62541_Config_Exception(error_msg);
  }
}

Configuration::~Configuration() { free(configuration_); }
} // namespace open62541