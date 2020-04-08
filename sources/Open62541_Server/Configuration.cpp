#include "Configuration.hpp"
#include "Config_Serializer.hpp"
#include "HaSLLLogger.hpp"
#include "LoggerRepository.hpp"

#include <open62541/network_tcp.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/pki_default.h>
#include <open62541/plugin/securitypolicy_default.h>

#include <open62541/server_config_default.h>

using namespace std;
using namespace HaSLL;
using namespace open62541;

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

void Configuration::initialiseConfiguration() {
  UA_BuildInfo_deleteMembers(&configuration_->buildInfo);
  UA_ApplicationDescription_deleteMembers(
      &configuration_->applicationDescription);
  for (size_t i = 0; i > configuration_->networkLayersSize; i++) {
    configuration_->networkLayers[i].deleteMembers(
        &configuration_->networkLayers[i]);
  }
  free(configuration_->networkLayers);
  configuration_->networkLayers = NULL;
  configuration_->networkLayersSize = 0;
  UA_String_deleteMembers(&configuration_->customHostname);
  configuration_->customHostname = UA_STRING_NULL;

  for (size_t i = 0; i < configuration_->securityPoliciesSize; ++i) {
    UA_SecurityPolicy *policy = &configuration_->securityPolicies[i];
    policy->deleteMembers(policy);
  }
  free(configuration_->securityPolicies);
  configuration_->securityPolicies = NULL;
  configuration_->securityPoliciesSize = 0;

  for (size_t i = 0; i < configuration_->endpointsSize; ++i)
    UA_EndpointDescription_deleteMembers(&configuration_->endpoints[i]);

  free(configuration_->endpoints);
  configuration_->endpoints = NULL;
  configuration_->endpointsSize = 0;

  if (configuration_->certificateVerification.deleteMembers)
    configuration_->certificateVerification.deleteMembers(
        &configuration_->certificateVerification);

  if (configuration_->accessControl.deleteMembers)
    configuration_->accessControl.deleteMembers(&configuration_->accessControl);

  if (configuration_->logger.clear)
    configuration_->logger.clear(configuration_->logger.context);
  memset(configuration_, 0, sizeof(UA_ServerConfig));
}

void Configuration::setupBasics() {
  configuration_->nThreads = config_file_.thread_count;
  configuration_->logger = HaSLL_Logger_;
  configuration_->shutdownDelay = config_file_.shutdown_delay_ms;
  configuration_->verifyRequestTimestamp = config_file_.rules_handling;
  configuration_->relaxEmptyValueConstraint = true;
  configuration_->nodeLifecycle.constructor = NULL;
  configuration_->nodeLifecycle.destructor = NULL;
  configuration_->nodeLifecycle.createOptionalChild = NULL;
  configuration_->nodeLifecycle.generateChildNodeId = NULL;
}

void Configuration::setupMetaInformation() {
  configuration_->buildInfo = config_file_.build_info;
  configuration_->applicationDescription = config_file_.app_info;
}

void Configuration::setupCertificate() {
  configuration_->serverCertificate = config_file_.server_certificate;
  UA_CertificateVerification_AcceptAll(
      &configuration_->certificateVerification);
}

void Configuration::setupSecureChannelLimits() {
  configuration_->maxSecureChannels =
      config_file_.secure_channels_limits.max_secure_channels;
  configuration_->maxSecurityTokenLifetime =
      config_file_.secure_channels_limits.max_security_token_lifetime_ms;
}

void Configuration::setupSessionLimits() {
  configuration_->maxSessions = config_file_.session_limits.max_sessions;
  configuration_->maxSessionTimeout =
      config_file_.session_limits.max_session_timeout_ms;
}

void Configuration::setupSubscriptionLimits() {
  configuration_->maxSubscriptions =
      config_file_.subscription_limits.max_subscriptions;
  configuration_->maxSubscriptionsPerSession =
      config_file_.subscription_limits.max_subscriptions_per_session;
  configuration_->publishingIntervalLimits =
      config_file_.subscription_limits.publishing_interval_limits_ms;
  configuration_->lifeTimeCountLimits =
      config_file_.subscription_limits.life_time_count_limits;
  configuration_->keepAliveCountLimits =
      config_file_.subscription_limits.keep_alive_count_limits;
  configuration_->maxNotificationsPerPublish =
      config_file_.subscription_limits.max_notifications_per_publish;
  configuration_->enableRetransmissionQueue =
      config_file_.subscription_limits.enable_retransmission_queue;
  configuration_->maxRetransmissionQueueSize =
      config_file_.subscription_limits.max_retransmission_queue_size;
  // configuration_->maxEventsPerNode =
  // config_file_.subscription_limits.max_events_per_node;
}

void Configuration::setupMonitoredItemLimits() {
  configuration_->maxMonitoredItems =
      config_file_.monitored_items_limits.max_monitored_items;
  configuration_->maxMonitoredItemsPerSubscription =
      config_file_.monitored_items_limits.max_monitored_items_per_subscription;
  configuration_->samplingIntervalLimits =
      config_file_.monitored_items_limits.sampling_interval_limits_ms;
  configuration_->queueSizeLimits =
      config_file_.monitored_items_limits.queue_size_limits;
}

void Configuration::setupOperationalLimits() {
  configuration_->maxNodesPerRead =
      config_file_.operation_limits.max_nodes_per_read;
  configuration_->maxNodesPerWrite =
      config_file_.operation_limits.max_nodes_per_write;
  configuration_->maxNodesPerMethodCall =
      config_file_.operation_limits.max_nodes_per_method_call;
  configuration_->maxNodesPerBrowse =
      config_file_.operation_limits.max_nodes_per_browse;
  configuration_->maxNodesPerRegisterNodes =
      config_file_.operation_limits.max_nodes_per_register_nodes;
  configuration_->maxNodesPerTranslateBrowsePathsToNodeIds =
      config_file_.operation_limits
          .max_nodes_per_translate_browse_paths_to_nodeids;
  configuration_->maxNodesPerNodeManagement =
      config_file_.operation_limits.max_nodes_per_node_management;
  configuration_->maxMonitoredItemsPerCall =
      config_file_.operation_limits.max_nodes_per_method_call;
}

UA_StatusCode Configuration::setupSecurityPolicies() {
  UA_StatusCode status;
  // switch (config_file_.security_policy) {
  // case SecurityPolicy::BASIC128_RSA15: {
  //   status = setupSecurityPolicies_BASIC128_RSA15();
  //   break;
  // }
  // case SecurityPolicy::BASIC256: {
  //   status = setupSecurityPolicies_BASIC256();
  //   break;
  // }
  // case SecurityPolicy::BASIC256_SHA256: {
  //   status = setupSecurityPolicies_BASIC256_SHA256();
  //   break;
  // }
  // case SecurityPolicy::ALL: {
  //   status = setupSecurityPolicies_None();
  //   status = setupSecurityPolicies_BASIC128_RSA15();
  //   status = setupSecurityPolicies_BASIC256();
  //   status = setupSecurityPolicies_BASIC256_SHA256();
  //   break;
  // }
  // case SecurityPolicy::NONE:
  // default: {
  //   status = setupSecurityPolicies_None();
  //   break;
  // }
  // }
  status = setupSecurityPolicies_None();
  return status;
}

UA_StatusCode Configuration::setupSecurityPolicies_None() {
  UA_StatusCode retval;

  /* Allocate the SecurityPolicies */
  UA_SecurityPolicy *tmp = (UA_SecurityPolicy *)UA_realloc(
      configuration_->securityPolicies,
      sizeof(UA_SecurityPolicy) * (1 + configuration_->securityPoliciesSize));
  if (!tmp)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  configuration_->securityPolicies = tmp;

  /* Populate the SecurityPolicies */
  UA_ByteString localCertificate = UA_BYTESTRING_NULL;
  if (config_file_.server_certificate.length)
    localCertificate = config_file_.server_certificate;
  retval = UA_SecurityPolicy_None(
      &configuration_->securityPolicies[configuration_->securityPoliciesSize],
      NULL, localCertificate, &configuration_->logger);
  if (retval != UA_STATUSCODE_GOOD)
    return retval;
  configuration_->securityPoliciesSize++;

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode Configuration::setupSecurityPolicies_BASIC128_RSA15() {
  // UA_StatusCode retval;

  // /* Allocate the SecurityPolicies */
  // UA_SecurityPolicy *tmp = (UA_SecurityPolicy *)UA_realloc(
  //     configuration_->securityPolicies,
  //     sizeof(UA_SecurityPolicy) * (1 +
  //     configuration_->securityPoliciesSize));
  // if (!tmp)
  //   return UA_STATUSCODE_BADOUTOFMEMORY;
  // configuration_->securityPolicies = tmp;

  // /* Populate the SecurityPolicies */
  // UA_ByteString localCertificate = UA_BYTESTRING_NULL;
  // UA_ByteString localPrivateKey = UA_BYTESTRING_NULL;
  // if (certificate)
  //   localCertificate = *certificate;
  // if (privateKey)
  //   localPrivateKey = *privateKey;
  // retval = UA_SecurityPolicy_Basic128Rsa15(
  //     &config_file_->securityPolicies[config_file_->securityPoliciesSize],
  //     &config_file_->certificateVerification, localCertificate,
  //     localPrivateKey, &config_file_->logger);
  // if (retval != UA_STATUSCODE_GOOD)
  //   return retval;
  // config_file_->securityPoliciesSize++;

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode Configuration::setupSecurityPolicies_BASIC256() {
  // UA_StatusCode retval;

  // /* Allocate the SecurityPolicies */
  // UA_SecurityPolicy *tmp = (UA_SecurityPolicy *)UA_realloc(
  //     config_file_->securityPolicies,
  //     sizeof(UA_SecurityPolicy) * (1 + config_file_->securityPoliciesSize));
  // if (!tmp)
  //   return UA_STATUSCODE_BADOUTOFMEMORY;
  // config_file_->securityPolicies = tmp;

  // /* Populate the SecurityPolicies */
  // UA_ByteString localCertificate = UA_BYTESTRING_NULL;
  // UA_ByteString localPrivateKey = UA_BYTESTRING_NULL;
  // if (certificate)
  //   localCertificate = *certificate;
  // if (privateKey)
  //   localPrivateKey = *privateKey;
  // retval = UA_SecurityPolicy_Basic256(
  //     &config_file_->securityPolicies[config_file_->securityPoliciesSize],
  //     &config_file_->certificateVerification, localCertificate,
  //     localPrivateKey, &config_file_->logger);
  // if (retval != UA_STATUSCODE_GOOD)
  //   return retval;
  // config_file_->securityPoliciesSize++;

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode Configuration::setupSecurityPolicies_BASIC256_SHA256() {
  // UA_StatusCode retval;

  // /* Allocate the SecurityPolicies */
  // UA_SecurityPolicy *tmp = (UA_SecurityPolicy *)UA_realloc(
  //     config_file_->securityPolicies,
  //     sizeof(UA_SecurityPolicy) * (1 + config_file_->securityPoliciesSize));
  // if (!tmp)
  //   return UA_STATUSCODE_BADOUTOFMEMORY;
  // config_file_->securityPolicies = tmp;

  // /* Populate the SecurityPolicies */
  // UA_ByteString localCertificate = UA_BYTESTRING_NULL;
  // UA_ByteString localPrivateKey = UA_BYTESTRING_NULL;
  // if (certificate)
  //   localCertificate = *certificate;
  // if (privateKey)
  //   localPrivateKey = *privateKey;
  // retval = UA_SecurityPolicy_Basic256Sha256(
  //     &config_file_->securityPolicies[config_file_->securityPoliciesSize],
  //     &config_file_->certificateVerification, localCertificate,
  //     localPrivateKey, &config_file_->logger);
  // if (retval != UA_STATUSCODE_GOOD)
  //   return retval;
  // config_file_->securityPoliciesSize++;

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode Configuration::setupNetowrkingLayer() {
  /* Add a network layer */
  UA_ServerNetworkLayer *tmp = (UA_ServerNetworkLayer *)realloc(
      configuration_->networkLayers,
      sizeof(UA_ServerNetworkLayer) * (1 + configuration_->networkLayersSize));
  if (!tmp)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  configuration_->networkLayers = tmp;

  UA_ConnectionConfig connection_config = config_file_.networking;
  configuration_->networkLayers[configuration_->networkLayersSize] =
      UA_ServerNetworkLayerTCP(connection_config, config_file_.port_nubmer,
                               &configuration_->logger);
  if (!configuration_->networkLayers[configuration_->networkLayersSize].handle)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  configuration_->networkLayersSize++;

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode Configuration::setupAccessControl() {
  UA_StatusCode retval;
  UA_UsernamePasswordLogin login_credentials = {
      config_file_.access_credentials.username,
      config_file_.access_credentials.password};
  retval = UA_AccessControl_default(
      configuration_, config_file_.allow_annonymous_access,
      &configuration_
           ->securityPolicies[configuration_->securityPoliciesSize - 1]
           .policyUri,
      1, &login_credentials);
  if (retval != UA_STATUSCODE_GOOD) {
    UA_ServerConfig_clean(configuration_);
  }
  return retval;
}

UA_StatusCode
Configuration::createEndpoint(UA_EndpointDescription *endpoint,
                              const UA_SecurityPolicy *securityPolicy,
                              UA_MessageSecurityMode securityMode) {
  UA_EndpointDescription_init(endpoint);

  endpoint->securityMode = securityMode;
  UA_String_copy(&securityPolicy->policyUri, &endpoint->securityPolicyUri);
  endpoint->transportProfileUri = UA_STRING_ALLOC(
      "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary");

  /* Add security level value for the corresponding message security mode */
  endpoint->securityLevel = (UA_Byte)securityMode;

  /* Enable all login mechanisms from the access control plugin  */
  UA_StatusCode retval =
      UA_Array_copy(configuration_->accessControl.userTokenPolicies,
                    configuration_->accessControl.userTokenPoliciesSize,
                    (void **)&endpoint->userIdentityTokens,
                    &UA_TYPES[UA_TYPES_USERTOKENPOLICY]);
  if (retval != UA_STATUSCODE_GOOD)
    return retval;
  endpoint->userIdentityTokensSize =
      configuration_->accessControl.userTokenPoliciesSize;

  UA_String_copy(&securityPolicy->localCertificate,
                 &endpoint->serverCertificate);
  UA_ApplicationDescription_copy(&configuration_->applicationDescription,
                                 &endpoint->server);

  return UA_STATUSCODE_GOOD;
}

// UA_ServerConfig_addEndpoint(UA_ServerConfig *config, const UA_String
// securityPolicyUri, UA_MessageSecurityMode securityMode)
UA_StatusCode Configuration::setupEndpoint() {
  UA_StatusCode retval;

  /* Allocate the endpoint */
  UA_EndpointDescription *tmp = (UA_EndpointDescription *)UA_realloc(
      configuration_->endpoints,
      sizeof(UA_EndpointDescription) * (1 + configuration_->endpointsSize));
  if (!tmp) {
    return UA_STATUSCODE_BADOUTOFMEMORY;
  }
  configuration_->endpoints = tmp;

  /* Lookup the security policy */
  const UA_SecurityPolicy *policy = NULL;
  for (size_t i = 0; i < configuration_->securityPoliciesSize; ++i) {
    if (UA_String_equal(&UA_SECURITY_POLICY_NONE_URI,
                        &configuration_->securityPolicies[i].policyUri)) {
      policy = &configuration_->securityPolicies[i];
      break;
    }
  }
  if (!policy)
    return UA_STATUSCODE_BADINVALIDARGUMENT;

  /* Populate the endpoint */
  retval =
      createEndpoint(&configuration_->endpoints[configuration_->endpointsSize],
                     policy, UA_MESSAGESECURITYMODE_NONE);
  if (retval != UA_STATUSCODE_GOOD)
    return retval;
  configuration_->endpointsSize++;

  return UA_STATUSCODE_GOOD;
}

UA_StatusCode Configuration::setupEndpoints() {
  UA_StatusCode retval;

  /* Allocate the endpoints */
  UA_EndpointDescription *tmp = (UA_EndpointDescription *)UA_realloc(
      configuration_->endpoints, sizeof(UA_EndpointDescription) *
                                     (2 * configuration_->securityPoliciesSize +
                                      configuration_->endpointsSize));
  if (!tmp) {
    return UA_STATUSCODE_BADOUTOFMEMORY;
  }
  configuration_->endpoints = tmp;

  /* Populate the endpoints */
  for (size_t i = 0; i < configuration_->securityPoliciesSize; ++i) {
    if (UA_String_equal(&UA_SECURITY_POLICY_NONE_URI,
                        &configuration_->securityPolicies[i].policyUri)) {
      retval = createEndpoint(
          &configuration_->endpoints[configuration_->endpointsSize],
          &configuration_->securityPolicies[i], UA_MESSAGESECURITYMODE_NONE);
      if (retval != UA_STATUSCODE_GOOD)
        return retval;
      configuration_->endpointsSize++;
    } else {
      retval = createEndpoint(
          &configuration_->endpoints[configuration_->endpointsSize],
          &configuration_->securityPolicies[i], UA_MESSAGESECURITYMODE_SIGN);
      if (retval != UA_STATUSCODE_GOOD)
        return retval;
      configuration_->endpointsSize++;

      retval = createEndpoint(
          &configuration_->endpoints[configuration_->endpointsSize],
          &configuration_->securityPolicies[i],
          UA_MESSAGESECURITYMODE_SIGNANDENCRYPT);
      if (retval != UA_STATUSCODE_GOOD)
        return retval;
      configuration_->endpointsSize++;
    }
  }
  return UA_STATUSCODE_GOOD;
}