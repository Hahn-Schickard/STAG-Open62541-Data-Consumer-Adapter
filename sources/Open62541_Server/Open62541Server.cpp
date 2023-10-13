#include "Open62541Server.hpp"

#include "Config_Serializer.hpp"
#include "HaSLL/LoggerManager.hpp"

using namespace std;
using namespace HaSLI;
using namespace Information_Model;
using namespace open62541;

Open62541Server::Open62541Server()
    : Open62541Server(make_unique<Configuration>()) {}

Open62541Server::Open62541Server(std::unique_ptr<Configuration> configuration)
    : logger_(LoggerManager::registerTypedLogger(this)) {
#ifdef UA_ENABLE_HISTORIZING
  historizer_ = configuration->obtainHistorizer();
#endif // UA_ENABLE_HISTORIZING
  auto config = configuration->getConfig();
  /* Config is consumed, so no need to save it
   * Inside UA_Server_newWithConfig assigns the config as follows
   *      server->config = *config;
   * and afterwards sets it to 0 with:
   *      memset(config, 0, sizeof(UA_ServerConfig));
   * this mimics cpp std::move()
   */
  open62541_server_ = UA_Server_newWithConfig(config.get());
  server_namespace_index_ = 1;
}

Open62541Server::~Open62541Server() { UA_Server_delete(open62541_server_); }

bool Open62541Server::start() {
  try {
    logger_->log(SeverityLevel::INFO, "Starting open62541 server!");
    if (isRunning()) {
      stop();
    }
    is_running_ = true;
    server_thread_ = thread(&Open62541Server::runnable, this);

    if (server_thread_.joinable()) {
      logger_->log(SeverityLevel::TRACE, "Open62541 server thread is running!");
      return true;
    } else {
      logger_->log(
          SeverityLevel::ERROR, "Could not start Open62541 server thread!");
      return false;
    }
  } catch (exception& ex) {
    logger_->log(SeverityLevel::CRITICAL, "Caught an exception: []", ex.what());
    return false;
  }
}

bool Open62541Server::stop() {
  if (isRunning()) {
    logger_->log(SeverityLevel::INFO, "Stopping open62541 server!");
    is_running_ = false;
    server_thread_.join();
    logger_->log(SeverityLevel::TRACE, "Joined open62541 server thread!");
    UA_StatusCode status = UA_Server_run_shutdown(open62541_server_);
    if (status == UA_STATUSCODE_GOOD) {
      logger_->log(SeverityLevel::TRACE, "open62541 server shutdown!");
      return true;
    } else {
      logger_->log(
          SeverityLevel::ERROR, "Could not shutdown open62541 server!");
    }
  }
  logger_->log(SeverityLevel::INFO, "Stopped open62541 server!");
  return false;
}

void Open62541Server::runnable() {
  try {
    UA_StatusCode status = UA_Server_run(open62541_server_, &is_running_);
    if (status != UA_STATUSCODE_GOOD) {
      logger_->log(SeverityLevel::ERROR,
          "ERROR:{} Failed to start open62541 server thread!", status);
    }
  } catch (exception& ex) {
    logger_->log(SeverityLevel::CRITICAL, "Caught an exception: []", ex.what());
  }
}

UA_UInt16 Open62541Server::getServerNamespace() const {
  return server_namespace_index_;
}

const UA_Logger* Open62541Server::getServerLogger() const {
  auto* config = UA_Server_getConfig(open62541_server_);
  return &config->logger;
}

UA_Server* Open62541Server::getServer() { return open62541_server_; }

bool Open62541Server::isRunning() {
  lock_guard<mutex> lock(status_mutex_);
  return is_running_;
}

#ifdef UA_ENABLE_HISTORIZING
UA_StatusCode Open62541Server::registerForHistorization(
    UA_NodeId node_id, const UA_DataType* type) {
  if (historizer_) {
    return historizer_->registerNodeId(open62541_server_, node_id, type);
  } else {
    return UA_STATUSCODE_BADRESOURCEUNAVAILABLE;
  }
}
#endif // UA_ENABLE_HISTORIZING