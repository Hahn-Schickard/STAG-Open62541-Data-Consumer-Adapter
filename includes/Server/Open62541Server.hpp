#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Configuration.hpp"

#include <HaSLL/Logger.hpp>
#include <Information_Model/Device.hpp>
#include <open62541/server.h>

#include <memory>
#include <mutex>
#include <thread>

namespace open62541 {

struct Open62541Server {
  Open62541Server(UA_ServerConfig* config);

  ~Open62541Server();

  bool start();

  bool stop();

  bool isRunning();

  UA_Server* getServer();

private:
  void runnable();

  volatile bool is_running_ = false;
  UA_Server* open62541_server_;
  std::thread server_thread_;
  HaSLL::LoggerPtr logger_;
  std::mutex status_mutex_;
};
} // namespace open62541
#endif //_OPEN62541_SERVER_H_