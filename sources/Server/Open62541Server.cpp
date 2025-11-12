#include "Open62541Server.hpp"

#include <HaSLL/LoggerManager.hpp>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace open62541;

Open62541Server::Open62541Server(UA_ServerConfig* config)
    : logger_(LoggerManager::registerLogger("Open62541::Runner")) {
  open62541_server_ = UA_Server_newWithConfig(config);
}

Open62541Server::~Open62541Server() { UA_Server_delete(open62541_server_); }

bool Open62541Server::start() {
  try {
    logger_->info("Starting open62541 server!");
    if (isRunning()) {
      stop();
    }
    is_running_ = true;
    server_thread_ = thread(&Open62541Server::runnable, this);

    if (server_thread_.joinable()) {
      logger_->trace("Open62541 server thread is running!");
      return true;
    } else {
      logger_->error("Could not start Open62541 server thread!");
      return false;
    }
  } catch (const exception& ex) {
    logger_->critical(
        "Caught an exception while starting the server: {}", ex.what());
    return false;
  }
}

bool Open62541Server::stop() {
  if (isRunning()) {
    logger_->info("Stopping open62541 server!");
    is_running_ = false;
    server_thread_.join();
    logger_->trace("Joined open62541 server thread!");
  }
  return true;
}

void Open62541Server::runnable() {
  do {
    try {
      logger_->info("Starting open62541 server thread");
      UA_StatusCode status = UA_Server_run(open62541_server_, &is_running_);
      if (status != UA_STATUSCODE_GOOD) {
        logger_->error("ERROR:{} Failed to start open62541 server thread!",
            UA_StatusCode_name(status));
      }
    } catch (const exception& ex) {
      logger_->critical(
          "Caught an exception during server lifetime: {}", ex.what());
    }
  } while (isRunning());
}

UA_Server* Open62541Server::getServer() { return open62541_server_; }

bool Open62541Server::isRunning() {
  lock_guard<mutex> lock(status_mutex_);
  return is_running_;
}
