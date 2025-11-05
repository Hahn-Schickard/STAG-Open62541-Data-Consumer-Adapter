#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Configuration.hpp"
#include "HaSLL/Logger.hpp"
#ifdef UA_ENABLE_HISTORIZING
#include "Historizer.hpp"
#endif // UA_ENABLE_HISTORIZING

#include <Information_Model/Device.hpp>
#include <open62541/server.h>

#include <memory>
#include <mutex>
#include <thread>

namespace open62541 {

class Open62541Server {
  volatile bool is_running_ = false;
  UA_Server* open62541_server_;
  UA_UInt16 server_namespace_index_;
  std::thread server_thread_;
  HaSLL::LoggerPtr logger_;
  std::mutex status_mutex_;
#ifdef UA_ENABLE_HISTORIZING
  std::unique_ptr<Historizer> historizer_;

  UA_StatusCode registerForHistorization(
      UA_NodeId node_id, const UA_DataType* type);
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
  UA_UInt16 getServerNamespace() const;
  const UA_Logger* getServerLogger() const;
  UA_Server* getServer();
};
} // namespace open62541
#endif //_OPEN62541_SERVER_H_