#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Config.hpp"
#include "Configuration.hpp"
#include "HaSLL/Logger.hpp"
#include "Information_Model/Device.hpp"
#ifdef UA_ENABLE_HISTORIZING
#include "Historizer.hpp"
#endif // UA_ENABLE_HISTORIZING

#include <memory>
#include <mutex>
#include <open62541/server.h>
#include <thread>

namespace open62541 {

class Open62541Server {
  volatile bool is_running_ = false;
  UA_Server* open62541_server_;
  UA_UInt16 server_namespace_index_;
  std::thread server_thread_;
  HaSLI::LoggerPtr logger_;
  std::mutex status_mutex_;
#ifdef UA_ENABLE_HISTORIZING
  std::unique_ptr<Historizer> historizer_;

  UA_StatusCode registerForHistorization(
      UA_NodeId nodeId, const UA_DataType* type);
#endif // UA_ENABLE_HISTORIZING

  void runnable();

  friend class NodeBuilder;

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
  const UA_Logger* getServerLogger();
  UA_Server* getServer();
};
} // namespace open62541
#endif //_OPEN62541_SERVER_H_