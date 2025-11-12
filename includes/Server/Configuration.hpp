#ifndef __OPEN62541_SERVER_CONFIGURATION_HPP_
#define __OPEN62541_SERVER_CONFIGURATION_HPP_

#ifdef ENABLE_UA_HISTORIZING
#include "Historizer.hpp"
#endif // ENABLE_UA_HISTORIZING

#include <HaSLL/Logger.hpp>
#include <open62541/server.h>

#include <filesystem>
#include <memory>
#include <stdexcept>

namespace open62541 {
struct Configuration {
  Configuration();
  explicit Configuration(const std::filesystem::path& filepath);
  ~Configuration() = default;

  std::unique_ptr<UA_ServerConfig> getConfig();
#ifdef ENABLE_UA_HISTORIZING
  HistorizerPtr getHistorizer() const;
#endif // ENABLE_UA_HISTORIZING

private:
  HaSLL::LoggerPtr logger_;
#ifdef ENABLE_UA_HISTORIZING
  HistorizerPtr historizer_;
#endif // ENABLE_UA_HISTORIZING
  std::unique_ptr<UA_ServerConfig> configuration_;
};
} // namespace open62541

#endif //__OPEN62541_SERVER_CONFIGURATION_HPP_