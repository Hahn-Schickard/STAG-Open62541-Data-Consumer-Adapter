#include "Config_Serializer.hpp"

#include <exception>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>

using namespace std;
using namespace open62541;

#define STRINGIFY(arg) #arg
#define VERSION(MAJOR, MINOR, PATCH, LABEL) \
  STRINGIFY(MAJOR) "." STRINGIFY(MINOR) "." STRINGIFY(PATCH) LABEL

UA_UInt32Range initUInt32Range(UA_UInt32 min, UA_UInt32 max) {
  UA_UInt32Range range = {min, max};
  return range;
}

UA_DurationRange initDurationRange(UA_Duration min, UA_Duration max) {
  UA_DurationRange range = {min, max};
  return range;
}

Config makeDefaultConfig() {
  // clang-format off
  UA_ConnectionConfig networking = {
    .protocolVersion = 0,
    .recvBufferSize = 65535, // NOLINT(readability-magic-numbers)
    .sendBufferSize = 65535, // NOLINT(readability-magic-numbers)
    .localMaxMessageSize = 0, // unlimited
    .remoteMaxMessageSize = 0, // unlimited
    .localMaxChunkCount = 0, // unlimited
    .remoteMaxChunkCount = 0 // unlimited
  };
  UA_BuildInfo build_info = {
      .productUri = UA_String_fromChars("http://open62541.org"),
      .manufacturerName = UA_String_fromChars("open62541"),
      .productName = UA_String_fromChars("open62541 OPC UA Server"),
      .softwareVersion = UA_String_fromChars(VERSION(
          ADAPTER_VERSION_MAJOR, ADAPTER_VERSION_MINOR,
          ADAPTER_VERSION_PATCH, ADAPTER_VERSION_LABEL)),
      .buildNumber = UA_String_fromChars(__DATE__ " " __TIME__), 
      .buildDate = UA_DateTime_now()
    };
  UA_ApplicationDescription app_info = {
      .applicationUri = UA_String_fromChars(
        "urn:open62541.server.application"),
      .productUri = UA_String_fromChars("http://open62541.org"),
      .applicationName = UA_LOCALIZEDTEXT_ALLOC(
        "en", "open62541-based OPC UA Application"),
      .applicationType = 
        UA_ApplicationType::UA_APPLICATIONTYPE_SERVER,
      .gatewayServerUri = UA_STRING_NULL,
      .discoveryProfileUri = UA_STRING_NULL,
      .discoveryUrlsSize = 0,
      .discoveryUrls = nullptr
  };
  SecureChannelsLimits secure_channels_limits = {
    // NOLINTNEXTLINE(readability-magic-numbers)
      .max_secure_channels = 40, 
    // NOLINTNEXTLINE(readability-magic-numbers)
      .max_security_token_lifetime_ms = 600000 /* 10 minutes */
  };
  SessionsLimits session_limits = {
    // NOLINTNEXTLINE(readability-magic-numbers)
      .max_sessions = 100, 
    // NOLINTNEXTLINE(readability-magic-numbers)
      .max_session_timeout_ms = 3600000 /* 1h */
  };
  OperationalLimits operation_limits = {};
  SubscriptionsLimits subscription_limits = {
      .max_subscriptions = 0, // unlimited
      .max_subscriptions_per_session = 0, // unlimited
      .publishing_interval_limits_ms = initDurationRange(
    //    100ms to 1h
          100.0, 3600000.0), // NOLINT(readability-magic-numbers)
    // NOLINTNEXTLINE(readability-magic-numbers)
      .life_time_count_limits = initUInt32Range(3, 15000),
    // NOLINTNEXTLINE(readability-magic-numbers)
      .keep_alive_count_limits = initUInt32Range(1, 100),
    // NOLINTNEXTLINE(readability-magic-numbers)
      .max_notifications_per_publish = 1000,
      .enable_retransmission_queue = true,
      .max_retransmission_queue_size = 0, // unlimited
      .max_events_per_node = 0  // unlimited
  };
  MonitoredItemsLimits monitored_items_limits = {
      .max_monitored_items = 0, // unlimited
      .max_monitored_items_per_subscription = 0, // unlimited
      .sampling_interval_limits_ms = initDurationRange(
    //    50ms to 24h
          50.0, 86400000.0), // NOLINT(readability-magic-numbers)
    // NOLINTNEXTLINE(readability-magic-numbers)
      .queue_size_limits = initUInt32Range(1, 100)
  };
  UA_MdnsDiscoveryConfiguration mdsnc_config = {
    .mdnsServerName = UA_STRING_NULL,
    .serverCapabilitiesSize = 0, 
    .serverCapabilities = nullptr
  };
  UA_ServerConfig_Discovery discovery = {
    .discoveryCleanupTimeout = 0, 
    .mdnsEnabled = false, 
    .mdnsConfig = mdsnc_config, 
    .mdnsInterfaceIP = UA_STRING_NULL, 
    .mdnsIpAddressListSize = 0, 
    .mdnsIpAddressList = nullptr
  };
  UserCredentials user_credentials = {};
  Historization historization = {
    .dsn = "PostgreSQL",
    .user =  "",
    .auth =  "", 
  // NOLINTNEXTLINE(readability-magic-numbers)
    .request_timeout = 60, // 1 min
    .request_logging = false
  };

  Config config = {
      .allow_anonymous_access = true,
      .access_credentials = user_credentials,
      .thread_count = 1,
      .port_number = 8888, //NOLINT(readability-magic-numbers)
      .networking = networking,
      .security_policy = SecurityPolicy::NONE,
      .build_info = build_info,
      .app_info = app_info,
      .server_certificate = UA_STRING_NULL,
      .shutdown_delay_ms = 0.0,
      .rules_handling = UA_RuleHandling::UA_RULEHANDLING_DEFAULT,
      .secure_channels_limits = secure_channels_limits,
      .session_limits = session_limits,
      .operation_limits = operation_limits,
      .max_references_per_node = 0,
      .subscription_limits = subscription_limits,
      .monitored_items_limits = monitored_items_limits,
      .max_publish_req_per_session = 0,
      .discovery = discovery,
      .historization = historization
  };
  // clang-format on
  return config;
}

// NOLINTNEXTLINE
NLOHMANN_JSON_SERIALIZE_ENUM(SecurityPolicy,
    {{NONE, "none"}, {BASIC128_RSA15, "Basic128_RSA15"}, {BASIC256, "Basic256"},
        {BASIC256_SHA256, "Basic256_Sha256"}, {ALL, "All"}})

// NOLINTNEXTLINE
NLOHMANN_JSON_SERIALIZE_ENUM(UA_ApplicationType,
    {{UA_APPLICATIONTYPE_SERVER, "Server"},
        {UA_APPLICATIONTYPE_CLIENT, "Client"},
        {UA_APPLICATIONTYPE_CLIENTANDSERVER, "Client and Server"},
        {UA_APPLICATIONTYPE_DISCOVERYSERVER, "Discovery Server"}})

// NOLINTNEXTLINE
NLOHMANN_JSON_SERIALIZE_ENUM(UA_RuleHandling,
    {{UA_RULEHANDLING_DEFAULT, "default"}, {UA_RULEHANDLING_ABORT, "abort"},
        {UA_RULEHANDLING_WARN, "warn"}, {UA_RULEHANDLING_ACCEPT, "accept"}})

namespace nlohmann {
// ======================== UA_String ============================
// The caller is responsible for deallocation of `p`
// NOLINTNEXTLINE
static void from_json(const json& j, UA_String& p) {
  string text = j.get<string>();
  if (!text.empty()) {
    p.length = strlen(text.c_str());
    p.data = (UA_Byte*)malloc(p.length);
    memcpy(p.data, text.c_str(), p.length);
  } else {
    p = UA_STRING_NULL;
  }
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_String& p) {
  string text = string((char*)p.data, p.length);
  j = text;
}

// ======================== UA_LocalizedText ============================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_LocalizedText& p) {
  p.locale = j.at("locale").get<UA_String>();
  p.text = j.at("text").get<UA_String>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_LocalizedText& p) {
  j = json{// clang-format off
      {"locale", p.locale},
      {"text", p.text}
  }; // clang-format on
}

// ======================== UA_DurationRange ============================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_DurationRange& p) {
  p.min = j.at("min").get<UA_Duration>();
  p.max = j.at("max").get<UA_Duration>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_DurationRange& p) {
  j = json{// clang-format off
      {"min", p.min},
      {"max", p.max}
  }; // clang-format on
}

// ======================== UA_UInt32Range ============================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_UInt32Range& p) {
  p.min = j.at("min").get<UA_UInt32>();
  p.max = j.at("max").get<UA_UInt32>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_UInt32Range& p) {
  j = json{// clang-format off
      {"min", p.min},
      {"max", p.max}
  }; // clang-format on
}

// ======================== SecureChannelsLimts ============================
// The caller is responsible for deallocation of `p`
// NOLINTNEXTLINE
static void from_json(const json& j, UserCredentials& p) {
  p.username = j.at("username").get<UA_String>();
  p.password = j.at("password").get<UA_String>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UserCredentials& p) {
  j = json{// clang-format off
      {"username", p.username},
      {"password", p.password}
  }; // clang-format on
}

// ======================== SecureChannelsLimts ============================
// NOLINTNEXTLINE
static void from_json(const json& j, SecureChannelsLimits& p) {
  p.max_secure_channels = j.at("max_secure_channels").get<UA_UInt16>();
  p.max_security_token_lifetime_ms =
      j.at("max_security_token_lifetime_ms").get<UA_UInt32>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const SecureChannelsLimits& p) {
  j = json{// clang-format off
      {"max_secure_channels", p.max_secure_channels},
      {"max_security_token_lifetime_ms", p.max_security_token_lifetime_ms}
  }; // clang-format on
}

// ============================ SessionsLimits ===============================
// NOLINTNEXTLINE
static void from_json(const json& j, SessionsLimits& p) {
  p.max_sessions = j.at("max_sessions").get<UA_UInt16>();
  p.max_session_timeout_ms = j.at("max_session_timeout_ms").get<UA_Double>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const SessionsLimits& p) {
  j = json{// clang-format off
      {"max_sessions", p.max_sessions},
      {"max_session_timeout_ms", p.max_session_timeout_ms}
  }; // clang-format on
}

// ======================== OperationalLimits ===========================
// NOLINTNEXTLINE
static void from_json(const json& j, OperationalLimits& p) {
  p.max_nodes_per_read = j.at("max_nodes_per_read").get<UA_UInt32>();
  p.max_nodes_per_write = j.at("max_nodes_per_write").get<UA_UInt32>();
  p.max_nodes_per_method_call =
      j.at("max_nodes_per_method_call").get<UA_UInt32>();
  p.max_nodes_per_browse = j.at("max_nodes_per_browse").get<UA_UInt32>();
  p.max_nodes_per_register_nodes =
      j.at("max_nodes_per_register_nodes").get<UA_UInt32>();
  p.max_nodes_per_translate_browse_paths_to_nodeids =
      j.at("max_nodes_per_translate_browse_paths_to_nodeids").get<UA_UInt32>();
  p.max_nodes_per_node_management =
      j.at("max_nodes_per_node_management").get<UA_UInt32>();
  p.max_monitored_items_per_call =
      j.at("max_monitored_items_per_call").get<UA_UInt32>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const OperationalLimits& p) {
  j = json{// clang-format off
      {"max_nodes_per_read", p.max_nodes_per_read},
      {"max_nodes_per_write", p.max_nodes_per_write},
      {"max_nodes_per_method_call", p.max_nodes_per_method_call},
      {"max_nodes_per_browse", p.max_nodes_per_browse},
      {"max_nodes_per_register_nodes", p.max_nodes_per_register_nodes},
      {"max_nodes_per_translate_browse_paths_to_nodeids",
          p.max_nodes_per_translate_browse_paths_to_nodeids},
      {"max_nodes_per_node_management", p.max_nodes_per_node_management},
      {"max_monitored_items_per_call", p.max_monitored_items_per_call}
  }; // clang-format on
}

// ======================== SubscriptionsLimtis ===========================
// NOLINTNEXTLINE
static void from_json(const json& j, SubscriptionsLimits& p) {
  p.max_subscriptions = j.at("max_subscriptions").get<UA_UInt32>();
  p.max_subscriptions_per_session =
      j.at("max_subscriptions_per_session").get<UA_UInt32>();
  p.publishing_interval_limits_ms =
      j.at("publishing_interval_limits_ms").get<UA_DurationRange>();
  p.life_time_count_limits =
      j.at("life_time_count_limits").get<UA_UInt32Range>();
  p.keep_alive_count_limits =
      j.at("keep_alive_count_limits").get<UA_UInt32Range>();
  p.max_notifications_per_publish =
      j.at("max_notifications_per_publish").get<UA_UInt32>();
  p.enable_retransmission_queue =
      j.at("enable_retransmission_queue").get<UA_Boolean>();
  p.max_retransmission_queue_size =
      j.at("max_retransmission_queue_size").get<UA_UInt32>();
  p.max_events_per_node = j.at("max_events_per_node").get<UA_UInt32>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const SubscriptionsLimits& p) {
  j = json{// clang-format off
      {"max_subscriptions", p.max_subscriptions},
      {"max_subscriptions_per_session", p.max_subscriptions_per_session},
      {"publishing_interval_limits_ms", p.publishing_interval_limits_ms},
      {"life_time_count_limits", p.life_time_count_limits},
      {"keep_alive_count_limits", p.keep_alive_count_limits},
      {"max_notifications_per_publish", p.max_notifications_per_publish},
      {"enable_retransmission_queue", p.enable_retransmission_queue},
      {"max_retransmission_queue_size", p.max_retransmission_queue_size},
      {"max_events_per_node", p.max_events_per_node}
  }; // clang-format on
}

// ======================== MonitoredItemsLimits ===========================
// NOLINTNEXTLINE
static void from_json(const json& j, MonitoredItemsLimits& p) {
  p.max_monitored_items = j.at("max_monitored_items").get<UA_UInt32>();
  p.max_monitored_items_per_subscription =
      j.at("max_monitored_items_per_subscription").get<UA_UInt32>();
  p.sampling_interval_limits_ms =
      j.at("sampling_interval_limits_ms").get<UA_DurationRange>();
  p.queue_size_limits = j.at("queue_size_limits").get<UA_UInt32Range>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const MonitoredItemsLimits& p) {
  j = json{// clang-format off
      {"max_monitored_items", p.max_monitored_items},
      {"max_monitored_items_per_subscription", 
          p.max_monitored_items_per_subscription},
      {"sampling_interval_limits_ms", p.sampling_interval_limits_ms},
      {"queue_size_limits", p.queue_size_limits}
  }; // clang-format on
}

// ======================== UA_ConnectionConfig ===========================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_ConnectionConfig& p) {
  p.protocolVersion = j.at("protocolVersion").get<UA_UInt32>();
  p.recvBufferSize = j.at("recvBufferSize").get<UA_UInt32>();
  p.sendBufferSize = j.at("sendBufferSize").get<UA_UInt32>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_ConnectionConfig& p) {
  j = json{// clang-format off
      {"protocolVersion", p.protocolVersion},
      {"recvBufferSize", p.recvBufferSize},
      {"sendBufferSize", p.sendBufferSize}
  }; // clang-format on
}

// ======================== UA_BuildInfo ===========================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_BuildInfo& p) {
  p.productUri = j.at("productUri").get<UA_String>();
  p.manufacturerName = j.at("manufacturerName").get<UA_String>();
  p.productName = j.at("productName").get<UA_String>();
  p.softwareVersion = j.at("softwareVersion").get<UA_String>();
  p.buildNumber = j.at("buildNumber").get<UA_String>();
  p.buildDate = j.at("buildDate").get<UA_DateTime>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_BuildInfo& p) {
  j = json{// clang-format off
      {"productUri", p.productUri},
      {"manufacturerName", p.manufacturerName}, 
      {"productName", p.productName},
      {"softwareVersion", p.softwareVersion}, 
      {"buildNumber", p.buildNumber},
      {"buildDate", p.buildDate}
  }; // clang-format on
}

// ====================== UA_ApplicationDescription =======================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_ApplicationDescription& p) {
  p.applicationUri = j.at("applicationUri").get<UA_String>();
  p.productUri = j.at("productUri").get<UA_String>();
  p.applicationName = j.at("applicationName").get<UA_LocalizedText>();
  p.applicationType = j.at("applicationType").get<UA_ApplicationType>();
  p.gatewayServerUri = j.at("gatewayServerUri").get<UA_String>();
  p.discoveryProfileUri = j.at("discoveryProfileUri").get<UA_String>();
  p.discoveryUrlsSize = 0; // @TODO: handle discovery urls
  p.discoveryUrls = nullptr;
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_ApplicationDescription& p) {
  j = json{// clang-format off
      {"applicationUri", p.applicationUri}, 
      {"productUri", p.productUri},
      {"applicationName", p.applicationName},
      {"applicationType", p.applicationType},
      {"gatewayServerUri", p.gatewayServerUri},
      {"discoveryProfileUri", p.discoveryProfileUri}
  }; // clang-format on
}

// ==================== UA_MdnsDiscoveryConfiguration =====================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_MdnsDiscoveryConfiguration& p) {
  p.mdnsServerName = j.at("mdnsServerName").get<UA_String>();
  //@TODO: handle server capabilities list
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_MdnsDiscoveryConfiguration& p) {
  j = json{// clang-format off
      {"mdnsServerName", p.mdnsServerName}
  }; // clang-format on
}

// ====================== UA_ServerConfig_Discovery =======================
// NOLINTNEXTLINE
static void from_json(const json& j, UA_ServerConfig_Discovery& p) {
  p.discoveryCleanupTimeout = j.at("cleanupTimeout").get<UA_UInt32>();
  p.mdnsEnabled = j.at("mdnsEnable").get<bool>();
}

// NOLINTNEXTLINE
static void to_json(json& j, const UA_ServerConfig_Discovery& p) {
  j = json{// clang-format off
      {"cleanupTimeout", p.discoveryCleanupTimeout},
      {"mdnsEnable", p.mdnsEnabled}
  }; // clang-format on
}

// ====================== Historization =======================
// NOLINTNEXTLINE
static void from_json(const json& j, optional<Historization>& p) {
  Historization h;
  h.dsn = j.at("DataSourceName").get<string>();
  h.user = j.at("Username").get<string>();
  h.auth = j.at("Password").get<string>();
  h.request_timeout = j.at("RequestTimeoutInSeconds").get<size_t>();
  h.request_logging = j.at("LogRequests").get<bool>();
  p = h;
}

// NOLINTNEXTLINE
static void to_json(json& j, const optional<Historization>& p) {
  if (p.has_value()) {
    auto h = p.value();
    j = json{// clang-format off
      {"DataSourceName", h.dsn}, 
      {"Username", h.user},
      {"Password", h.auth}, 
      {"RequestTimeoutInSeconds", h.request_timeout},
      {"LogRequests", h.request_logging}
  }; // clang-format on
  }
}

// ======================== Config ===========================
// The caller is responsible for deallocation of `p`
// NOLINTNEXTLINE
static void from_json(const json& j, Config& p) {
  p.allow_anonymous_access = j.at("allow_anonymous_access").get<UA_Boolean>();
  p.access_credentials = j.at("access_credentials").get<UserCredentials>();
  p.port_number = j.at("port_number").get<UA_UInt16>();
  p.networking = j.at("networking").get<UA_ConnectionConfig>();
  p.security_policy = j.at("security_policy").get<SecurityPolicy>();
  p.build_info = j.at("build_info").get<UA_BuildInfo>();
  p.app_info = j.at("app_info").get<UA_ApplicationDescription>();
  p.server_certificate = j.at("server_certificate").get<UA_ByteString>();
  p.shutdown_delay_ms = j.at("shutdown_delay_ms").get<UA_Double>();
  p.rules_handling = j.at("rules_handling").get<UA_RuleHandling>();
  p.secure_channels_limits =
      j.at("secure_channels_limits").get<SecureChannelsLimits>();
  p.session_limits = j.at("session_limits").get<SessionsLimits>();
  p.operation_limits = j.at("operation_limits").get<OperationalLimits>();
  p.max_references_per_node = j.at("max_references_per_node").get<UA_UInt32>();
  p.subscription_limits =
      j.at("subscription_limits").get<SubscriptionsLimits>();
  p.monitored_items_limits =
      j.at("monitored_items_limits").get<MonitoredItemsLimits>();
  p.max_publish_req_per_session =
      j.at("max_publish_req_per_session").get<UA_UInt32>();
  p.discovery = j.at("discovery").get<UA_ServerConfig_Discovery>();
  if (auto it = j.find("historization"); it != j.end()) {
    p.historization = j.at("historization").get<optional<Historization>>();
  } else {
    p.historization = nullopt;
  }
}

// NOLINTNEXTLINE
static void to_json(json& j, const Config& p) {
  j = json{// clang-format off
      {"allow_anonymous_access", p.allow_anonymous_access},
      {"access_credentials", p.access_credentials},
      {"port_number", p.port_number}, 
      {"networking", p.networking},
      {"security_policy", p.security_policy}, 
      {"build_info", p.build_info},
      {"app_info", p.app_info}, 
      {"server_certificate", p.server_certificate},
      {"shutdown_delay_ms", p.shutdown_delay_ms},
      {"rules_handling", p.rules_handling},
      {"secure_channels_limits", p.secure_channels_limits},
      {"session_limits", p.session_limits},
      {"operation_limits", p.operation_limits},
      {"max_references_per_node", p.max_references_per_node},
      {"subscription_limits", p.subscription_limits},
      {"monitored_items_limits", p.monitored_items_limits},
      {"max_publish_req_per_session", p.max_publish_req_per_session},
      {"discovery", p.discovery}, 
      {"historization", p.historization}
  }; // clang-format on
}
} // namespace nlohmann

// The caller is responsible for deallocating the result
Config open62541::deserializeConfig(const string& file_path) {
  ifstream input_file_stream(file_path);
  if (input_file_stream) {
    nlohmann::json j;
    input_file_stream >> j;
    return j.get<Config>();
  } else {
    return makeDefaultConfig();
  }
}

void open62541::serializeConfig(const string& file_path, const Config& config) {
  ofstream output_file_stream(file_path);
  if (output_file_stream) {
    nlohmann::json j = config;

    output_file_stream << j << endl;
  }
}
