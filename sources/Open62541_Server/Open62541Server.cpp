#include "Open62541Server.hpp"

#include "LoggerRepository.hpp"
#include "ServerConfig.hpp"

#include <open62541/server.h>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;

Open62541Server::Open62541Server()
    : is_running_(false)
    , logger_(LoggerRepository::getInstance().registerTypedLoger(this)) {
  configure();
  open62541_server_       = UA_Server_newWithConfig(config_);
  server_namespace_index_ = 1;
}

Open62541Server::~Open62541Server() {
  free(config_);
}

bool Open62541Server::configure() {
  logger_->log(
      SeverityLevel::TRACE, "Setting up configuration file for open62541!");
  config_ = (UA_ServerConfig*) malloc(sizeof(UA_ServerConfig));

  memset(config_, 0, sizeof(UA_ServerConfig));

  UA_StatusCode status = HS_ServerConfig_setDefault(config_);
  if(status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "Failled reading configuration file for open62541!");
    return false;
  } else {
    logger_->log(
        SeverityLevel::TRACE, "Configuration file for open62541 configured!");
    return true;
  }
}

bool Open62541Server::start() {
  logger_->log(SeverityLevel::INFO, "Starting open62541 server!");
  if(isRunning()) {
    stop();
  }
  is_running_    = true;
  server_thread_ = thread(&Open62541Server::runnable, this);

  if(server_thread_.joinable()) {
    logger_->log(SeverityLevel::TRACE, "Open62541 server thread is running!");
    return true;
  } else {
    logger_->log(
        SeverityLevel::ERROR, "Could not start Open62541 server thread!");
    return false;
  }
}

bool Open62541Server::stop() {
  if(isRunning()) {
    logger_->log(SeverityLevel::INFO, "Stopping open62541 server!");
    is_running_ = false;
    server_thread_.join();
    logger_->log(SeverityLevel::TRACE, "Joined open62541 server thread!");
    UA_StatusCode status = UA_Server_run_shutdown(open62541_server_);
    if(status == UA_STATUSCODE_GOOD) {
      UA_Server_delete(open62541_server_);
      logger_->log(SeverityLevel::TRACE, "Cleaned up open62541 server!");
      return true;
    } else {
      logger_->log(
          SeverityLevel::ERROR, "Could not clean up open62541 server!");
    }
  }
  logger_->log(SeverityLevel::INFO, "Stopped open62541 server!");
  return false;
}

void Open62541Server::runnable() {
  UA_StatusCode status = UA_Server_run(open62541_server_, &is_running_);
  if(status != UA_STATUSCODE_GOOD) {
    logger_->log(SeverityLevel::ERROR,
        "ERROR:{} Failed to start open62541 server thread!",
        status);
  }
}

UA_UInt16 Open62541Server::getServerNamespace() {
  return server_namespace_index_;
}

UA_Server* Open62541Server::getServer() {
  return open62541_server_;
}

bool Open62541Server::isRunning() {
  lock_guard<mutex> lock(status_mutex_);
  return is_running_;
}