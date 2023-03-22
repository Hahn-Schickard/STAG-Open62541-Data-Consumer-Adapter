#include "Configuration.hpp"
#include "Config_Serializer.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL_Logger.hpp"
#include "Historizer.hpp"

#include <open62541/network_tcp.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/server_config_default.h>

using namespace std;
using namespace HaSLI;

namespace open62541 {
Configuration::Configuration() : Configuration(false) {}

Configuration::Configuration(bool basic)
    : logger_(LoggerManager::registerLogger("Open62541 Configuration")),
      configuration_(make_unique<UA_ServerConfig>()) {
  try {
    memset(configuration_.get(), 0, sizeof(UA_ServerConfig));
    configuration_->logger = HaSLL_Logger_;
    if (basic) {
      UA_ServerConfig_setBasics(configuration_.get());
    } else {
      UA_ServerConfig_setDefault(configuration_.get());
    }
  } catch (exception& ex) {
    string error_msg =
        "Caught exception when deserializing Configuration file: " +
        string(ex.what());
    throw Open62541_Config_Exception(error_msg);
  }
}

void addSecurityPolicy(UA_ServerConfig* config, SecurityPolicy policy) {
  auto localCertificate = UA_BYTESTRING_NULL;
  auto localPrivateKey = UA_BYTESTRING_NULL;

  switch (policy) {
  case NONE: {
    auto status =
        UA_ServerConfig_addSecurityPolicyNone(config, &localCertificate);
    if (status != UA_STATUSCODE_GOOD) {
      string error_msg = "Failed to set SecurityPolicy#None due to error " +
          string(UA_StatusCode_name(status));
      throw Open62541_Config_Exception(error_msg);
    }
    break;
  }
#ifdef UA_ENABLE_ENCRYPTION
  case BASIC128_RSA15: {
    UA_ServerConfig_addSecurityPolicyBasic128Rsa15(
        config, &localCertificate, &localPrivateKey);
    if (status != UA_STATUSCODE_GOOD) {
      string error_msg =
          "Failed to set SecurityPolicy#Basic128Rsa15 due to error " +
          string(UA_StatusCode_name(status));
      throw Open62541_Config_Exception(error_msg);
    }
    break;
  }
  case BASIC256: {
    UA_ServerConfig_addSecurityPolicyBasic256(
        config, &localCertificate, &localPrivateKey);
    if (status != UA_STATUSCODE_GOOD) {
      string error_msg = "Failed to set SecurityPolicy#Basic256 due to error " +
          string(UA_StatusCode_name(status));
      throw Open62541_Config_Exception(error_msg);
    }
    break;
  }
  case BASIC256_SHA256: {
    UA_ServerConfig_addSecurityPolicyBasic256Sha256(
        config, &localCertificate, &localPrivateKey);
    if (status != UA_STATUSCODE_GOOD) {
      string error_msg =
          "Failed to set SecurityPolicy#Aes128Sha256RsaOaep due to error " +
          string(UA_StatusCode_name(status));
      throw Open62541_Config_Exception(error_msg);
    }
    break;
  }
  case AES128_SHA256_RSAO_AEP: {
    UA_ServerConfig_addSecurityPolicyAes128Sha256RsaOaep(
        config, &localCertificate, &localPrivateKey);
    if (status != UA_STATUSCODE_GOOD) {
      string error_msg =
          "Failed to set SecurityPolicy#Aes128Sha256RsaOaep due to error " +
          string(UA_StatusCode_name(status));
      throw Open62541_Config_Exception(error_msg);
    }
    break;
  }
#endif
  case ALL: {
    UA_StatusCode status;
#ifdef UA_ENABLE_ENCRYPTION
    status = UA_ServerConfig_addAllSecurityPolicies(
        config, &localCertificate, &localPrivateKey);
#else
    status = UA_ServerConfig_addSecurityPolicyNone(config, &localCertificate);
#endif
    if (status != UA_STATUSCODE_GOOD) {
      string error_msg = "Failed to set SecurityPolicy#All due to error " +
          string(UA_StatusCode_name(status));
      throw Open62541_Config_Exception(error_msg);
    }
    break;
  }
  default: { throw Open62541_Config_Exception("Unsupported security policy"); }
  }
}

Configuration::Configuration(const string& filepath) : Configuration(true) {
  auto config = deserializeConfig(filepath);

  configuration_->buildInfo = config.build_info;
  configuration_->applicationDescription = config.app_info;
  configuration_->serverCertificate = config.server_certificate;
  configuration_->shutdownDelay = config.shutdown_delay_ms;
  configuration_->verifyRequestTimestamp = config.rules_handling;

  configuration_->maxSecureChannels =
      config.secure_channels_limits.max_secure_channels;
  configuration_->maxSecurityTokenLifetime =
      config.secure_channels_limits.max_security_token_lifetime_ms;

  configuration_->maxSessions = config.session_limits.max_sessions;
  configuration_->maxSessionTimeout =
      config.session_limits.max_session_timeout_ms;

  configuration_->maxNodesPerRead = config.operation_limits.max_nodes_per_read;
  configuration_->maxNodesPerWrite =
      config.operation_limits.max_nodes_per_write;
  configuration_->maxNodesPerMethodCall =
      config.operation_limits.max_nodes_per_method_call;
  configuration_->maxNodesPerBrowse =
      config.operation_limits.max_nodes_per_browse;
  configuration_->maxNodesPerRegisterNodes =
      config.operation_limits.max_nodes_per_register_nodes;
  configuration_->maxNodesPerTranslateBrowsePathsToNodeIds =
      config.operation_limits.max_nodes_per_translate_browse_paths_to_nodeids;
  configuration_->maxNodesPerNodeManagement =
      config.operation_limits.max_nodes_per_node_management;
  configuration_->maxMonitoredItemsPerCall =
      config.operation_limits.max_monitored_items_per_call;

  configuration_->maxReferencesPerNode = config.max_references_per_node;

  configuration_->maxSubscriptions =
      config.subscription_limits.max_subscriptions;
  configuration_->maxSubscriptionsPerSession =
      config.subscription_limits.max_subscriptions_per_session;
  configuration_->publishingIntervalLimits =
      config.subscription_limits.publishing_interval_limits_ms;
  configuration_->lifeTimeCountLimits =
      config.subscription_limits.life_time_count_limits;
  configuration_->keepAliveCountLimits =
      config.subscription_limits.keep_alive_count_limits;
  configuration_->maxNotificationsPerPublish =
      config.subscription_limits.max_notifications_per_publish;
  configuration_->enableRetransmissionQueue =
      config.subscription_limits.enable_retransmission_queue;
  configuration_->maxRetransmissionQueueSize =
      config.subscription_limits.max_retransmission_queue_size;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
  configuration_->maxEventsPerNode =
      config.subscription_limits.max_events_per_node;
#endif

#ifdef UA_ENABLE_HISTORIZING
  try {
    /** @todo: obtain database configuration data and parse it to historizer
     */
    historizer_ = make_unique<Historizer>();
    configuration_->historyDatabase = historizer_->createDatabase();
    configuration_->accessHistoryDataCapability = true;
    configuration_->accessHistoryEventsCapability = false;
    configuration_->maxReturnDataValues = 0; // unlimited
    configuration_->insertDataCapability = false;
    configuration_->insertEventCapability = false;
    configuration_->insertAnnotationsCapability = false;
    configuration_->replaceDataCapability = false;
    configuration_->replaceEventCapability = false;
    configuration_->updateDataCapability = false;
    configuration_->updateEventCapability = false;
    configuration_->deleteRawCapability = false;
    configuration_->deleteEventCapability = false;
    configuration_->deleteAtTimeDataCapability = false;
  } catch (exception& ex) {
    logger_->error("Data Historization Service will not be available, due to "
                   "an exception: {}",
        ex.what());
  }
#endif

  configuration_->maxMonitoredItems =
      config.monitored_items_limits.max_monitored_items;
  configuration_->maxMonitoredItemsPerSubscription =
      config.monitored_items_limits.max_monitored_items_per_subscription;
  configuration_->samplingIntervalLimits =
      config.monitored_items_limits.sampling_interval_limits_ms;
  configuration_->queueSizeLimits =
      config.monitored_items_limits.queue_size_limits;

  configuration_->maxPublishReqPerSession = config.max_publish_req_per_session;
#ifdef UA_ENABLE_DISCOVERY
  configuration_->discoveryCleanupTimeout =
      config.discovery.discoveryCleanupTimeout;
#ifdef UA_ENABLE_DISCOVERY_MULTICAST
  configuration_->mdnsEnabled = config.discovery.mdnsEnabled;
  configuration_->mdnsConfig = config.discovery.mdnsConfig;
  configuration_->mdnsInterfaceIP = config.discovery.mdnsInterfaceIP;
#if !defined(UA_HAS_GETIFADDR)
  configuration_->mdnsIpAddressListSize =
      config.discovery.mdnsIpAddressListSize;
  configuration_->mdnsIpAddressList = config.discovery.mdnsIpAddressList;
#endif //! UA_HAS_GETIFADDR
#endif // UA_ENABLE_DISCOVERY_MULTICAST
#endif // UA_ENABLE_DISCOVERY

  try {
    UA_ServerConfig_addNetworkLayerTCP(configuration_.get(), config.port_number,
        config.networking.sendBufferSize, config.networking.recvBufferSize);
    addSecurityPolicy(configuration_.get(), config.security_policy);
    auto policy_uri =
        configuration_
            ->securityPolicies[configuration_->securityPoliciesSize - 1]
            .policyUri;
    UA_AccessControl_default(configuration_.get(), true, NULL, &policy_uri, 0,
        NULL); // allow_anonymous_access
    UA_ServerConfig_addEndpoint(configuration_.get(),
        UA_SECURITY_POLICY_NONE_URI, UA_MESSAGESECURITYMODE_NONE);
  } catch (exception& ex) {
    UA_ServerConfig_clean(configuration_.get());
    string error_msg = "Caught exception while creating Open62541 Config: " +
        string(ex.what());
    throw Open62541_Config_Exception(error_msg);
  }
} // namespace open62541

unique_ptr<UA_ServerConfig> Configuration::getConfig() {
  return move(configuration_);
}

unique_ptr<Historizer> Configuration::obtainHistorizer() {
  return move(historizer_);
}
} // namespace open62541