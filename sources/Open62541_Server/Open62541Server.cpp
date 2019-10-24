#include "Open62541Server.hpp"
#include "ServerConfig.hpp"

#include <open62541/server.h>

using namespace std;
using namespace Information_Model;

Open62541Server::Open62541Server() {
  is_running_ = false;
  configure();
  open62541_server_ = UA_Server_newWithConfig(config_);
  server_namespace_index_ = 1;
}

Open62541Server::~Open62541Server() {}

Open62541Server *Open62541Server::getInstance() {
  if (!instance_) {
    instance_ = new Open62541Server();
  }
  return instance_;
}

bool Open62541Server::configure() {
  config_ = (UA_ServerConfig *)malloc(sizeof(UA_ServerConfig));

  memset(config_, 0, sizeof(UA_ServerConfig));

  return HS_ServerConfig_setDefault(config_) == UA_STATUSCODE_GOOD ? true
                                                                   : false;
}

bool Open62541Server::start() {
  if (is_running_) {
    stop();
  }
  is_running_ = true;

  return pthread_create(&server_thread_id, NULL, &Open62541Server::runnable,
                        &instance_) == 0
             ? true
             : false;
}

bool Open62541Server::stop() {
  if (is_running_) {
    is_running_ = false;
    if (pthread_join(server_thread_id, NULL) == 0) {
      UA_StatusCode retval = UA_Server_run_shutdown(open62541_server_);
      if (retval == UA_STATUSCODE_GOOD) {
        UA_Server_delete(open62541_server_);
        return true;
      }
    }
  }
  return false;
}

void *Open62541Server::serverThread() {
  UA_Server_run(open62541_server_, &is_running_);
  return NULL;
}

void *Open62541Server::runnable(void *none) {
  return instance_->serverThread();
}

UA_UInt16 Open62541Server::getServerNamespace() {
  return server_namespace_index_;
}

UA_Server *Open62541Server::getServer() { return open62541_server_; }

Open62541Server *Open62541Server::instance_ = 0;