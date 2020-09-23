#include "Configuration.hpp"
#include "Config_Serializer.hpp"
#include "HaSLLLogger.hpp"
#include "LoggerRepository.hpp"

#include <open62541/network_tcp.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/pki_default.h>
#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/server_config.h>

#include <open62541/server_config_default.h>

using namespace std;
using namespace HaSLL;

namespace open62541 {
Configuration::Configuration(const std::string configuraiton_file_path)
    : logger_(LoggerRepository::getInstance().registerTypedLoger(this)),
      configuration_((UA_ServerConfig *)malloc(sizeof(UA_ServerConfig))) {
  try {
    config_file_ = deserializeConfig(configuraiton_file_path);
    // initialiseConfiguration();
    // setupBasics();
    // setupMetaInformation();
    // setupCertificate();
    // setupSecureChannelLimits();
    // setupSessionLimits();
    // setupSubscriptionLimits();
    // setupMonitoredItemLimits();
    // setupOperationalLimits();
    // setupNetowrkingLayer();
    // setupSecurityPolicies();
    // setupAccessControl();
    // setupEndpoint();
    UA_ServerConfig_setDefault(configuration_);
  } catch (exception &ex) {
    logger_->log(SeverityLevel::ERROR,
                 "Cought exception when deserializing Configuration file: []",
                 ex.what());
  }
}

Configuration::~Configuration() {
  try {
    free(configuration_);
  } catch (exception &ex) {
    logger_->log(SeverityLevel::ERROR,
                 "Cought exception during open62541 configuration cleanup: []",
                 ex.what());
  }
}
} // namespace open62541