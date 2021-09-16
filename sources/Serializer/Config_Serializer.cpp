#include "Config_Serializer.hpp"
#include "LoggerRepository.hpp"

#include <exception>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

using namespace std;
using namespace open62541;
using namespace HaSLL;

#define STRINGIFY(arg) #arg
#define VERSION(MAJOR, MINOR, PATCH, LABEL)                                    \
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
  UA_ConnectionConfig networking = {0, 65535, 65535, 0, 0};
  UA_BuildInfo build_info = {UA_String_fromChars("http://open62541.org"),
                             UA_String_fromChars("open62541"),
                             UA_String_fromChars("open62541 OPC UA Server"),
                             UA_String_fromChars(VERSION(
                                 ADAPTER_VERSION_MAJOR, ADAPTER_VERSION_MINOR,
                                 ADAPTER_VERSION_PATCH, ADAPTER_VERSION_LABEL)),
                             UA_String_fromChars(__DATE__ " " __TIME__),
                             UA_DateTime_now()};
  UA_ApplicationDescription app_info = {
      UA_String_fromChars("urn:open62541.server.application"),
      UA_String_fromChars("http://open62541.org"),
      UA_LOCALIZEDTEXT_ALLOC("en", "open62541-based OPC UA Application"),
      UA_ApplicationType::UA_APPLICATIONTYPE_SERVER,
      UA_STRING_NULL,
      UA_STRING_NULL,
      0,
      NULL,
  };
  SecureChannelsLimits secure_channels_limits = {
      40, (10 * 60 * 1000) /* 10 minutes */
  };
  SessionsLimits session_limits = {
      100, (60.0 * 60.0 * 1000.0) /* 1h */
  };
  OperationalLimits operation_limits = {};
  SubscriptionsLimits subscription_limits = {
      0,
      0,
      initDurationRange(100.0, (3600.0 * 1000.0)),
      initUInt32Range(3, 15000),
      initUInt32Range(1, 100),
      1000,
      true,
      0,
      0};
  MonitoredItemsLimits monitored_items_limits = {
      0, 0, initDurationRange(50.0, 24.0 * 3600.0 * 1000.0),
      initUInt32Range(1, 100)};
  UA_ServerConfig_Discovery discovery = {0, false};
  UserCredentials user_credentials = {};

  Config config = {
      true,
      user_credentials,
      1,
      8888,
      networking,
      SecurityPolicy::NONE,
      build_info,
      app_info,
      UA_STRING_NULL,
      0.0,
      UA_RuleHandling::UA_RULEHANDLING_DEFAULT,
      secure_channels_limits,
      session_limits,
      operation_limits,
      0,
      subscription_limits,
      monitored_items_limits,
      0,
      discovery,
  };
  return config;
}

NLOHMANN_JSON_SERIALIZE_ENUM(SecurityPolicy,
                             {{NONE, "none"},
                              {BASIC128_RSA15, "Basic128_RSA15"},
                              {BASIC256, "Basic256"},
                              {BASIC256_SHA256, "Basic256_Sha256"},
                              {ALL, "All"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
    UA_ApplicationType,
    {{UA_APPLICATIONTYPE_SERVER, "Server"},
     {UA_APPLICATIONTYPE_CLIENT, "Client"},
     {UA_APPLICATIONTYPE_CLIENTANDSERVER, "Client and Server"},
     {UA_APPLICATIONTYPE_DISCOVERYSERVER, "Discovery Server"}})

NLOHMANN_JSON_SERIALIZE_ENUM(UA_RuleHandling,
                             {{UA_RULEHANDLING_DEFAULT, "default"},
                              {UA_RULEHANDLING_ABORT, "abort"},
                              {UA_RULEHANDLING_WARN, "warn"},
                              {UA_RULEHANDLING_ACCEPT, "accept"}})

namespace nlohmann {
// ======================== UA_String ============================
static void from_json(const json &j, UA_String &p) {
  string text = j.get<string>();
  if (!text.empty()) {
    p.length = strlen(text.c_str());
    p.data = (UA_Byte *)malloc(p.length);
    memcpy(p.data, text.c_str(), p.length);
  } else {
    p = UA_STRING_NULL;
  }
}

static void to_json(json &j, const UA_String &p) {
  string text = string((char *)p.data, p.length);
  j = text;
}

// ======================== UA_LocalizedText ============================
static void from_json(const json &j, UA_LocalizedText &p) {
  p.locale = j.at("locale").get<UA_String>();
  p.text = j.at("text").get<UA_String>();
}

static void to_json(json &j, const UA_LocalizedText &p) {
  j = json{{"locale", p.locale}, {"text", p.text}};
}

// ======================== UA_DurationRange ============================
static void from_json(const json &j, UA_DurationRange &p) {
  p.min = j.at("min").get<UA_Duration>();
  p.max = j.at("max").get<UA_Duration>();
}

static void to_json(json &j, const UA_DurationRange &p) {
  j = json{{"min", p.min}, {"max", p.max}};
}

// ======================== UA_UInt32Range ============================
static void from_json(const json &j, UA_UInt32Range &p) {
  p.min = j.at("min").get<UA_UInt32>();
  p.max = j.at("max").get<UA_UInt32>();
}

static void to_json(json &j, const UA_UInt32Range &p) {
  j = json{{"min", p.min}, {"max", p.max}};
}

// ======================== SecureChannelsLimts ============================
static void from_json(const json &j, UserCredentials &p) {
  p.username = j.at("username").get<UA_String>();
  p.password = j.at("password").get<UA_String>();
}

static void to_json(json &j, const UserCredentials &p) {
  j = json{{"username", p.username}, {"password", p.password}};
}

// ======================== SecureChannelsLimts ============================
static void from_json(const json &j, SecureChannelsLimits &p) {
  p.max_secure_channels = j.at("max_secure_channels").get<UA_UInt16>();
  p.max_security_token_lifetime_ms =
      j.at("max_security_token_lifetime_ms").get<UA_UInt32>();
}

static void to_json(json &j, const SecureChannelsLimits &p) {
  j = json{
      {"max_secure_channels", p.max_secure_channels},
      {"max_security_token_lifetime_ms", p.max_security_token_lifetime_ms}};
}

// ============================ SessionsLimits ===============================
static void from_json(const json &j, SessionsLimits &p) {
  p.max_sessions = j.at("max_sessions").get<UA_UInt16>();
  p.max_session_timeout_ms = j.at("max_session_timeout_ms").get<UA_Double>();
}

static void to_json(json &j, const SessionsLimits &p) {
  j = json{{"max_sessions", p.max_sessions},
           {"max_session_timeout_ms", p.max_session_timeout_ms}};
}

// ======================== OperationalLimits ===========================
static void from_json(const json &j, OperationalLimits &p) {
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

static void to_json(json &j, const OperationalLimits &p) {
  j = json{{"max_nodes_per_read", p.max_nodes_per_read},
           {"max_nodes_per_write", p.max_nodes_per_write},
           {"max_nodes_per_method_call", p.max_nodes_per_method_call},
           {"max_nodes_per_browse", p.max_nodes_per_browse},
           {"max_nodes_per_register_nodes", p.max_nodes_per_register_nodes},
           {"max_nodes_per_translate_browse_paths_to_nodeids",
            p.max_nodes_per_translate_browse_paths_to_nodeids},
           {"max_nodes_per_node_management", p.max_nodes_per_node_management},
           {"max_monitored_items_per_call", p.max_monitored_items_per_call}};
}

// ======================== SubscriptionsLimtis ===========================
static void from_json(const json &j, SubscriptionsLimits &p) {
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

static void to_json(json &j, const SubscriptionsLimits &p) {
  j = json{{"max_subscriptions", p.max_subscriptions},
           {"max_subscriptions_per_session", p.max_subscriptions_per_session},
           {"publishing_interval_limits_ms", p.publishing_interval_limits_ms},
           {"life_time_count_limits", p.life_time_count_limits},
           {"keep_alive_count_limits", p.keep_alive_count_limits},
           {"max_notifications_per_publish", p.max_notifications_per_publish},
           {"enable_retransmission_queue", p.enable_retransmission_queue},
           {"max_retransmission_queue_size", p.max_retransmission_queue_size},
           {"max_events_per_node", p.max_events_per_node}};
}

// ======================== MonitoredItemsLimits ===========================
static void from_json(const json &j, MonitoredItemsLimits &p) {
  p.max_monitored_items = j.at("max_monitored_items").get<UA_UInt32>();
  p.max_monitored_items_per_subscription =
      j.at("max_monitored_items_per_subscription").get<UA_UInt32>();
  p.sampling_interval_limits_ms =
      j.at("sampling_interval_limits_ms").get<UA_DurationRange>();
  p.queue_size_limits = j.at("queue_size_limits").get<UA_UInt32Range>();
}

static void to_json(json &j, const MonitoredItemsLimits &p) {
  j = json{{"max_monitored_items", p.max_monitored_items},
           {"max_monitored_items_per_subscription",
            p.max_monitored_items_per_subscription},
           {"sampling_interval_limits_ms", p.sampling_interval_limits_ms},
           {"queue_size_limits", p.queue_size_limits}};
}

// ======================== UA_ConnectionConfig ===========================
static void from_json(const json &j, UA_ConnectionConfig &p) {
  p.protocolVersion = j.at("protocolVersion").get<UA_UInt32>();
  p.recvBufferSize = j.at("recvBufferSize").get<UA_UInt32>();
  p.sendBufferSize = j.at("sendBufferSize").get<UA_UInt32>();
}

static void to_json(json &j, const UA_ConnectionConfig &p) {
  j = json{{"protocolVersion", p.protocolVersion},
           {"recvBufferSize", p.recvBufferSize},
           {"sendBufferSize", p.sendBufferSize}};
}

// ======================== UA_BuildInfo ===========================
static void from_json(const json &j, UA_BuildInfo &p) {
  p.productUri = j.at("productUri").get<UA_String>();
  p.manufacturerName = j.at("manufacturerName").get<UA_String>();
  p.productName = j.at("productName").get<UA_String>();
  p.softwareVersion = j.at("softwareVersion").get<UA_String>();
  p.buildNumber = j.at("buildNumber").get<UA_String>();
  p.buildDate = j.at("buildDate").get<UA_DateTime>();
}

static void to_json(json &j, const UA_BuildInfo &p) {
  j = json{
      {"productUri", p.productUri},   {"manufacturerName", p.manufacturerName},
      {"productName", p.productName}, {"softwareVersion", p.softwareVersion},
      {"buildNumber", p.buildNumber}, {"buildDate", p.buildDate}};
}

// ====================== UA_ApplicationDescription =======================
static void from_json(const json &j, UA_ApplicationDescription &p) {
  p.applicationUri = j.at("applicationUri").get<UA_String>();
  p.productUri = j.at("productUri").get<UA_String>();
  p.applicationName = j.at("applicationName").get<UA_LocalizedText>();
  p.applicationType = j.at("applicationType").get<UA_ApplicationType>();
  p.gatewayServerUri = j.at("gatewayServerUri").get<UA_String>();
  p.discoveryProfileUri = j.at("discoveryProfileUri").get<UA_String>();
  p.discoveryUrlsSize = 0; // @TODO: handle discovery urls
  p.discoveryUrls = nullptr;
}

static void to_json(json &j, const UA_ApplicationDescription &p) {
  j = json{{"applicationUri", p.applicationUri},
           {"productUri", p.productUri},
           {"applicationName", p.applicationName},
           {"applicationType", p.applicationType},
           {"gatewayServerUri", p.gatewayServerUri},
           {"discoveryProfileUri", p.discoveryProfileUri}};
}

// ==================== UA_MdnsDiscoveryConfiguration =====================
static void from_json(const json &j, UA_MdnsDiscoveryConfiguration &p) {
  p.mdnsServerName = j.at("mdnsServerName").get<UA_String>();
  //@TODO: handle server capabilities list
}

static void to_json(json &j, const UA_MdnsDiscoveryConfiguration &p) {
  j = json{{"mdnsServerName", p.mdnsServerName}};
}

// ====================== UA_ServerConfig_Discovery =======================
static void from_json(const json &j, UA_ServerConfig_Discovery &p) {
  p.cleanupTimeout = j.at("cleanupTimeout").get<UA_UInt32>();
  p.mdnsEnable = j.at("mdnsEnable").get<bool>();
}

static void to_json(json &j, const UA_ServerConfig_Discovery &p) {
  j = json{{"cleanupTimeout", p.cleanupTimeout}, {"mdnsEnable", p.mdnsEnable}};
}

// ======================== Config ===========================
static void from_json(const json &j, Config &p) {
  p.allow_anonymous_access = j.at("allow_anonymous_access").get<UA_Boolean>();
  p.access_credentials = j.at("access_credentials").get<UserCredentials>();
  p.thread_count = j.at("thread_count").get<UA_UInt16>();
  p.port_nubmer = j.at("port_nubmer").get<UA_UInt16>();
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
}

static void to_json(json &j, const Config &p) {
  j = json{{"allow_anonymous_access", p.allow_anonymous_access},
           {"access_credentials", p.access_credentials},
           {"thread_count", p.thread_count},
           {"port_nubmer", p.port_nubmer},
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
           {"discovery", p.discovery}};
}
} // namespace nlohmann

const Config open62541::deserializeConfig(const string &file_path) {
  Config config = makeDefaultConfig();
  ifstream input_file_stream(file_path);
  if (input_file_stream) {
    nlohmann::json j;
    input_file_stream >> j;
    config = j.get<Config>();
  }
  return config;
}

void open62541::serializeConfig(const string &file_path, const Config &config) {
  ofstream output_file_stream(file_path);
  if (output_file_stream) {
    nlohmann::json j = config;

    output_file_stream << j << endl;
  }
}
