#ifndef __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_
#define __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_

#include "Config.hpp"
#include "Logger.hpp"

#include <memory>
#include <open62541/server.h>
#include <string>

namespace open62541 {
class Configuration {
  std::shared_ptr<HaSLL::Logger> logger_;
  Config config_file_;
  UA_ServerConfig *configuration_;

  UA_StatusCode createEndpoint(UA_EndpointDescription *endpoint,
                               const UA_SecurityPolicy *securityPolicy,
                               UA_MessageSecurityMode securityMode);
  void initialiseConfiguration();
  void setupBasics();
  void setupMetaInformation();
  void setupCertificate();
  void setupSecureChannelLimits();
  void setupSessionLimits();
  void setupSubscriptionLimits();
  void setupMonitoredItemLimits();
  void setupOperationalLimits();
  // All of the ones bellow should throw expections on failure and return void
  UA_StatusCode setupNetowrkingLayer();
  UA_StatusCode setupSecurityPolicies();
  UA_StatusCode setupSecurityPolicies_None(); // Review the implementations
  UA_StatusCode
  setupSecurityPolicies_BASIC128_RSA15();         // Review the implementations
  UA_StatusCode setupSecurityPolicies_BASIC256(); // Review the implementations
  UA_StatusCode
  setupSecurityPolicies_BASIC256_SHA256(); // Review the implementations
  UA_StatusCode setupAccessControl();
  UA_StatusCode setupEndpoint();
  UA_StatusCode setupEndpoints();

public:
  Configuration(const std::string configuraiton_file_path);
  ~Configuration();

  const UA_ServerConfig *getConfig() const { return configuration_; }
};
} // namespace open62541

#endif //__DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_