#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Device.hpp"
#include "Logger.hpp"

#include <memory>
#include <mutex>
#include <open62541/server.h>
#include <thread>

namespace open62541 {
class Open62541Server {
public:
  Open62541Server();
  ~Open62541Server();

  bool start();
  bool stop();
  bool configure();
  bool isRunning();
  UA_UInt16 getServerNamespace();
  UA_Server *getServer();

private:
  void runnable();

  volatile bool is_running_;
  UA_ServerConfig *config_;
  UA_Server *open62541_server_;
  UA_UInt16 server_namespace_index_;
  std::thread server_thread_;
  std::shared_ptr<HaSLL::Logger> logger_;
  std::mutex status_mutex_;
};
} // namespace open62541
#endif //_OPEN62541_SERVER_H_