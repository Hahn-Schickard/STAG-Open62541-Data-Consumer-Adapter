#ifndef __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_
#define __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_

#include "Config.hpp"
#include "Logger.hpp"

#include <memory>
#include <open62541/server.h>
#include <string>

namespace open62541 {
class Configuration {
  std::shared_ptr<HaSLL::Logger> logger_;
  Config config_file_;
  UA_ServerConfig *configuration_;

public:
  Configuration(const std::string configuraiton_file_path);
  ~Configuration();

  const UA_ServerConfig *getConfig() const { return configuration_; }
};
} // namespace open62541

#endif //__DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_