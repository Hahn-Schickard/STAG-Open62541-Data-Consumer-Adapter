#include "Config_Serializer.hpp"
#include "Configuration.hpp"
#include "HaSLLLogger.hpp"

#include <open62541/network_tcp.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/securitypolicy_default.h>

using namespace std;

namespace open62541 {

Double_Use::Double_Use()
  : std::logic_error("Double use") {}

Configuration::Configuration() {
  try {
    configuration_ = make_unique<UA_ServerConfig>();
    *configuration_.get() = {};
    configuration_->logger.log = HaSLL_Logger_.log;
    configuration_->logger.clear = HaSLL_Logger_.clear;
    // @TODO: Implement Config.hpp and Config_Serializer.hpp usage to set
    // UA_Config
    UA_ServerConfig_setDefault(configuration_.get());
  } catch (exception &ex) {
    string error_msg =
        "Caught exception when deserializing Configuration file: " +
        string(ex.what());
    throw Open62541_Config_Exception(error_msg);
  }
}

Configuration::Configuration(const std::string & filepath)
  : Configuration()
{
  try {
    Config config = deserializeConfig(filepath);

    // TODO: allow_annonymous_access
    // TODO: access_credentials
    configuration_->nThreads = config.thread_count;

    configuration_->networkLayersSize = 1;
    configuration_->networkLayers = new UA_ServerNetworkLayer;
    configuration_->networkLayers[0] = UA_ServerNetworkLayerTCP(
      // TODO: is TCP correct, or WS?
      config.networking, config.port_nubmer, 0, &configuration_->logger);

/*
    switch (config.security_policy) {
      default:
        case NONE:
          configuration_->securityPoliciesSize = 1;
          configuration_->securityPolicies = new UA_SecurityPolicy;
          UA_SecurityPolicy_None(
            configuration_->securityPolicies,
            TODO,
            &configuration_->logger);
          break;
        // TODO: BASIC128_RSA15, BASIC256, BASIC256_SHA256, ALL
        throw Open62541_Config_Exception("Unsupported security policy");
    }
*/

    configuration_->buildInfo = config.build_info;
    configuration_->applicationDescription = config.app_info;
    configuration_->serverCertificate = config.server_certificate;
    configuration_->shutdownDelay = config.shutdown_delay_ms;
    configuration_->verifyRequestTimestamp = config.rules_handling;

    configuration_->maxSecureChannels
      = config.secure_channels_limits.max_secure_channels;
    configuration_->maxSecurityTokenLifetime
      = config.secure_channels_limits.max_security_token_lifetime_ms;

    configuration_->maxSessions = config.session_limits.max_sessions;
    configuration_->maxSessionTimeout
      = config.session_limits.max_session_timeout_ms;

    configuration_->maxNodesPerRead
      = config.operation_limits.max_nodes_per_read;
    configuration_->maxNodesPerWrite
      = config.operation_limits.max_nodes_per_write;
    configuration_->maxNodesPerMethodCall
      = config.operation_limits.max_nodes_per_method_call;
    configuration_->maxNodesPerBrowse
      = config.operation_limits.max_nodes_per_browse;
    configuration_->maxNodesPerRegisterNodes
      = config.operation_limits.max_nodes_per_register_nodes;
    configuration_->maxNodesPerTranslateBrowsePathsToNodeIds
      = config.operation_limits.max_nodes_per_translate_browse_paths_to_nodeids;
    configuration_->maxNodesPerNodeManagement
      = config.operation_limits.max_nodes_per_node_management;
    configuration_->maxMonitoredItemsPerCall
      = config.operation_limits.max_monitored_items_per_call;

    configuration_->maxReferencesPerNode = config.max_references_per_node;

    configuration_->maxSubscriptions
      = config.subscription_limits.max_subscriptions;
    configuration_->maxSubscriptionsPerSession
      = config.subscription_limits.max_subscriptions_per_session;
    configuration_->publishingIntervalLimits
      = config.subscription_limits.publishing_interval_limits_ms;
    configuration_->lifeTimeCountLimits
      = config.subscription_limits.life_time_count_limits;
    configuration_->keepAliveCountLimits
      = config.subscription_limits.keep_alive_count_limits;
    configuration_->maxNotificationsPerPublish
      = config.subscription_limits.max_notifications_per_publish;
    configuration_->enableRetransmissionQueue
      = config.subscription_limits.enable_retransmission_queue;
    configuration_->maxRetransmissionQueueSize
      = config.subscription_limits.max_retransmission_queue_size;

    // TODO: subscription_limits.max_events_per_node

    configuration_->maxMonitoredItems
      = config.monitored_items_limits.max_monitored_items;
    configuration_->maxMonitoredItemsPerSubscription
      = config.monitored_items_limits.max_monitored_items_per_subscription;
    configuration_->samplingIntervalLimits
      = config.monitored_items_limits.sampling_interval_limits_ms;
    configuration_->queueSizeLimits
      = config.monitored_items_limits.queue_size_limits;

    configuration_->maxPublishReqPerSession
      = config.max_publish_req_per_session;
    configuration_->discovery = config.discovery;
  } catch (exception &ex) {
    string error_msg =
        "Caught exception when deserializing Configuration file: " +
        string(ex.what());
    throw Open62541_Config_Exception(error_msg);
  }
}

std::unique_ptr<const UA_ServerConfig> Configuration::getConfig() {
  if (configuration_) {
    std::unique_ptr<UA_ServerConfig> ret;
    ret.swap(configuration_);
    return ret;
  } else
    throw Double_Use();
}

Configuration::~Configuration() {
  if (configuration_)
    UA_ServerConfig_clean(configuration_.get());
}

} // namespace open62541