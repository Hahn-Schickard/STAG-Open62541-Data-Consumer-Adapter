#ifndef _OPEN62541_SERVER_H_
#define _OPEN62541_SERVER_H_

#include "Device.hpp"

#include <memory>
#include <open62541/server.h>
#include <pthread.h>

class Open62541Server {
public:
  bool start();
  bool stop();
  bool configure();
  bool isRunning();
  ~Open62541Server();
  static Open62541Server *getInstance();
  UA_UInt16 getServerNamespace();
  UA_Server *getServer();

private:
  Open62541Server();
  static void *runnable(void *none);
  void *serverThread();

  volatile bool is_running_;
  static Open62541Server *instance_;
  UA_ServerConfig *config_;
  UA_Server *open62541_server_;
  UA_UInt16 server_namespace_index_;
  pthread_t server_thread_id;
};

#endif //_OPEN62541_SERVER_H_