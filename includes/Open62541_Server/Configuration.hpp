#ifndef __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_
#define __DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_

#include "HaSLL/Logger.hpp"
#ifdef UA_ENABLE_HISTORIZING
#include "Historizer.hpp"
#endif // UA_ENABLE_HISTORIZING

#include <memory>
#include <open62541/server.h>
#include <stdexcept>

namespace open62541 {
struct Open62541_Config_Exception : public std::runtime_error {
  Open62541_Config_Exception(std::string const& message)
      : std::runtime_error(message) {}
};

struct Configuration {
  Configuration();
  Configuration(const std::string& filepath);
  ~Configuration() = default;

  std::unique_ptr<UA_ServerConfig> getConfig();
#ifdef UA_ENABLE_HISTORIZING
  std::unique_ptr<Historizer> obtainHistorizer();
#endif // UA_ENABLE_HISTORIZING

private:
  Configuration(bool basic);

  HaSLL::LoggerPtr logger_;
#ifdef UA_ENABLE_HISTORIZING
  std::unique_ptr<Historizer> historizer_;
#endif // UA_ENABLE_HISTORIZING
  std::unique_ptr<UA_ServerConfig> configuration_;
};
} // namespace open62541

#endif //__DCAI_OPEN62541_SERVER_CONFIGURATION_HPP_