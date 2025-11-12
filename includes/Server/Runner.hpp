#ifndef __OPEN62541_RUNNER_HPP
#define __OPEN62541_RUNNER_HPP

#include "Configuration.hpp"

#include <HaSLL/Logger.hpp>
#include <Information_Model/Device.hpp>
#include <open62541/server.h>

#include <memory>
#include <thread>

namespace open62541 {

struct Runner {
  Runner(UA_ServerConfig* config);

  ~Runner();

  bool start();

  bool stop();

  bool isRunning() const;

  UA_Server* getServer();

private:
  void runnable();

  volatile bool running_ = false;
  UA_Server* server_;
  std::thread thread_;
  HaSLL::LoggerPtr logger_;
};
} // namespace open62541
#endif //__OPEN62541_RUNNER_HPP