#ifndef __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_
#define __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_

#include "HaSLL/Logger.hpp"
#include "Historizer.hpp"

#include <memory>
#include <open62541/server.h>
#include <stdexcept>

namespace open62541 {
struct Open62541_Config_Exception : public std::runtime_error {
  Open62541_Config_Exception(std::string const& message)
      : std::runtime_error(message) {}
};

struct Configuration {
  using UA_ServerConfigPtr =
      std::unique_ptr<UA_ServerConfig, void (*)(UA_ServerConfig*)>;

  Configuration();
  Configuration(const std::string& filepath);
  ~Configuration();

  UA_ServerConfigPtr getConfig();
  // may only be called once, throws Double_Use afterwards

  std::unique_ptr<Historizer> obtainHistorizer();

private:
  HaSLI::LoggerPtr logger_;
  std::unique_ptr<Historizer> historizer_;
  UA_ServerConfigPtr configuration_;
};
} // namespace open62541

#endif //__DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_