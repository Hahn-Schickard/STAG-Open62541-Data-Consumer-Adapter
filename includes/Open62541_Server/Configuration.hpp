#ifndef __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_
#define __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_

#include <open62541/server.h>
#include <stdexcept>

namespace open62541 {
struct Open62541_Config_Exception : public std::runtime_error {
  Open62541_Config_Exception(std::string const &message)
      : std::runtime_error(message) {}
};

class Configuration {
  UA_ServerConfig *configuration_;

public:
  Configuration();
  Configuration(const std::string & filepath);
  ~Configuration();

  const UA_ServerConfig *getConfig() const { return configuration_; }
};
} // namespace open62541

#endif //__DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_