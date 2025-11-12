#include "Configuration.hpp"
#include "CheckStatus.hpp"
#include "Logger.hpp"

#ifdef ENABLE_UA_HISTORIZING
#include "Historizer.hpp"
#endif // ENABLE_UA_HISTORIZING

#include <HaSLL/LoggerManager.hpp>
#include <open62541/server_config_default.h>
#include <open62541/server_config_file_based.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace open62541 {
using namespace std;
using namespace HaSLL;

Configuration::Configuration()
    : logger_(LoggerManager::registerLogger("Open62541::Configuration")),
      configuration_(make_unique<UA_ServerConfig>()) {
  memset(configuration_.get(), 0, sizeof(UA_ServerConfig));
  configuration_->logging = createHaSLL();
  auto status = UA_ServerConfig_setDefault(configuration_.get());
  checkStatusCode("While setting configuration defaults", status, true);
}

size_t checkPath(const filesystem::path& filepath) {
  if (!is_regular_file(filepath)) {
    throw runtime_error(
        "Path " + filepath.string() + " does not point to a file");
  }
  if (!exists(filepath)) {
    throw runtime_error("File " + filepath.string() + " does not exist");
  }
  auto filesize = filesystem::file_size(filepath);
  if (filesize == 0) {
    throw runtime_error("File " + filepath.string() + " is empty");
  }
  return filesize;
}

UA_ByteString readFile(const filesystem::path& filepath) {
  auto filesize = checkPath(filepath);

  UA_ByteString result = UA_BYTESTRING_NULL;
  UA_ByteString_allocBuffer(&result, filesize);

  ifstream file_descriptor(filepath, ios::binary);
  file_descriptor.read(
      reinterpret_cast<char*>(result.data), static_cast<long>(filesize));

  return result;
}

Configuration::Configuration(const filesystem::path& filepath)
    : Configuration() {
  UA_ByteString json_config = readFile(filepath);

  auto status =
      UA_ServerConfig_updateFromFile(configuration_.get(), json_config);
  checkStatusCode(
      "While reading configuration file " + filepath.string(), status, true);

#ifdef ENABLE_UA_HISTORIZING
  if (configuration_->historizingEnabled) {
    try {
      historizer_ = make_shared<Historizer>();
      configuration_->historyDatabase = createDatabaseStruct(historizer_);
    } catch (exception& ex) {
      logger_->error("Data Historization Service will not be available, due to "
                     "an exception: {}",
          ex.what());
    }
  }
#endif // ENABLE_UA_HISTORIZING
} // namespace open62541

unique_ptr<UA_ServerConfig> Configuration::getConfig() {
  return move(configuration_);
}

#ifdef ENABLE_UA_HISTORIZING
HistorizerPtr Configuration::getHistorizer() { return historizer_; }
#endif // ENABLE_UA_HISTORIZING

} // namespace open62541