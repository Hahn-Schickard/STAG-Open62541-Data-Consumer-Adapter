#include "Runner.hpp"

#include <HaSLL/LoggerManager.hpp>

namespace open62541 {
using namespace std;
using namespace HaSLL;
using namespace Information_Model;

Runner::Runner(UA_ServerConfig* config)
    : logger_(LoggerManager::registerLogger("Open62541::Runner")) {
  server_ = UA_Server_newWithConfig(config);
}

Runner::~Runner() { UA_Server_delete(server_); }

bool Runner::start() {
  try {
    logger_->info("Starting open62541 server!");
    if (isRunning()) {
      stop();
    }
    running_ = true;
    thread_ = thread(&Runner::runnable, this);

    if (thread_.joinable()) {
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

bool Runner::stop() {
  if (isRunning()) {
    logger_->info("Stopping open62541 server!");
    running_ = false;
    thread_.join();
    logger_->trace("Joined open62541 server thread!");
  }
  return true;
}

void Runner::runnable() {
  do {
    try {
      logger_->info("Starting open62541 server thread");
      auto status = UA_Server_run(server_, &running_);
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

UA_Server* Runner::getServer() { return server_; }

bool Runner::isRunning() const { return running_; }
} // namespace open62541