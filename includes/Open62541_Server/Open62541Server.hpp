#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Config.hpp"
#include "Configuration.hpp"
#include "Information_Model/Device.hpp"
#include "Logger.hpp"

#include <memory>
#include <mutex>
#include <open62541/server.h>
#include <thread>

namespace open62541 {
class Open62541Server {
  volatile bool is_running_;
  std::unique_ptr<const UA_ServerConfig> server_configuration_;
  UA_Server *open62541_server_;
  UA_UInt16 server_namespace_index_;
  std::thread server_thread_;
  std::shared_ptr<HaSLL::Logger> logger_;
  std::mutex status_mutex_;

  void runnable();

public:
  /**
   * @pre HaSLL::LoggerRepository has been initialized
   */
  Open62541Server();

  /**
   * @pre HaSLL::LoggerRepository has been initialized
   */
  Open62541Server(std::unique_ptr<Configuration>);

  ~Open62541Server();

  bool start();
  bool stop();
  bool isRunning();
  UA_UInt16 getServerNamespace();
  const UA_Logger *getServerLogger();
  UA_Server *getServer();
};
} // namespace open62541
#endif //_OPEN62541_SERVER_H_