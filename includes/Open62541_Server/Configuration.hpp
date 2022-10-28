#ifndef __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_
#define __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_

#include <memory>
#include <open62541/server.h>
#include <stdexcept>

namespace open62541 {
struct Open62541_Config_Exception : public std::runtime_error {
  Open62541_Config_Exception(std::string const& message)
      : std::runtime_error(message) {}
};

struct Double_Use : public std::logic_error {
  Double_Use();
};

class Configuration {
  std::unique_ptr<UA_ServerConfig> configuration_;

public:
  Configuration();
  Configuration(const std::string& filepath);
  ~Configuration();

  std::unique_ptr<const UA_ServerConfig> getConfig();
  // may only be called once, throws Double_Use afterwards
};
} // namespace open62541

#endif //__DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_