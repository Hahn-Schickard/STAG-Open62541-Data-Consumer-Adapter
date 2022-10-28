#ifndef __DCA_OPEN62541_CONFIG_HS_HPP
#define __DCA_OPEN62541_CONFIG_HS_HPP

#include <open62541/server_config.h>

#define ADAPTER_VERSION_MAJOR "0"
#define ADAPTER_VERSION_MINOR "0"
#define ADAPTER_VERSION_PATCH "0"
#define ADAPTER_VERSION_LABEL "-rc"

namespace open62541 {

typedef enum SecurityPolicyEnum {
  NONE,
  BASIC128_RSA15,
  BASIC256,
  BASIC256_SHA256,
  ALL
} SecurityPolicy;

// This needs to be hashed and read as a hash code with hash seed set via CMake
// Argument
typedef struct {
  UA_String username;
  UA_String password;
} UserCredentials;

typedef struct SecureChannelsLimitsStruct {
  UA_UInt16 max_secure_channels;
  UA_UInt32 max_security_token_lifetime_ms;
} SecureChannelsLimits;

typedef struct SessionsLimitsStruct {
  UA_UInt16 max_sessions;
  UA_Double max_session_timeout_ms;
} SessionsLimits;

typedef struct OperationalLimitsStruct {
  UA_UInt32 max_nodes_per_read;
  UA_UInt32 max_nodes_per_write;
  UA_UInt32 max_nodes_per_method_call;
  UA_UInt32 max_nodes_per_browse;
  UA_UInt32 max_nodes_per_register_nodes;
  UA_UInt32 max_nodes_per_translate_browse_paths_to_nodeids;
  UA_UInt32 max_nodes_per_node_management;
  UA_UInt32 max_monitored_items_per_call;
} OperationalLimits;

typedef struct SubscriptionsLimitsStruct {
  UA_UInt32 max_subscriptions;
  UA_UInt32 max_subscriptions_per_session;
  UA_DurationRange
      publishing_interval_limits_ms; /* in ms (must not be less than 5) */
  UA_UInt32Range life_time_count_limits;
  UA_UInt32Range keep_alive_count_limits;
  UA_UInt32 max_notifications_per_publish;
  UA_Boolean enable_retransmission_queue;
  UA_UInt32 max_retransmission_queue_size; /* 0 -> unlimited size */
  UA_UInt32 max_events_per_node; /* 0 -> unlimited size, optional */
} SubscriptionsLimits;

typedef struct MonitoredItemsLimitsStruct {
  UA_UInt32 max_monitored_items;
  UA_UInt32 max_monitored_items_per_subscription;
  UA_DurationRange
      sampling_interval_limits_ms; /* in ms (must not be less than 5) */
  UA_UInt32Range queue_size_limits; /* Negotiated with the client */
} MonitoredItemsLimits;

struct Config {
  UA_Boolean allow_anonymous_access;
  UserCredentials access_credentials;
  UA_UInt16 thread_count;
  UA_UInt16 port_number;
  UA_ConnectionConfig networking;
  SecurityPolicy security_policy;
  UA_BuildInfo build_info;
  UA_ApplicationDescription app_info;
  UA_ByteString server_certificate;
  UA_Double shutdown_delay_ms;
  UA_RuleHandling rules_handling;
  SecureChannelsLimits secure_channels_limits;
  SessionsLimits session_limits;
  OperationalLimits operation_limits;
  UA_UInt32 max_references_per_node;
  SubscriptionsLimits subscription_limits;
  MonitoredItemsLimits monitored_items_limits;
  UA_UInt32 max_publish_req_per_session;
  UA_ServerConfig_Discovery discovery;
};
} // namespace open62541

#endif //__DCA_OPEN62541_CONFIG_HS_HPP