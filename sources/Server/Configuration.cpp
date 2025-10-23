#include "Configuration.hpp"
#include "HaSLL_Logger.hpp"
#include "Historizer.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <open62541/server_config_default.h>
#include <open62541/server_config_file_based.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace open62541 {
using namespace std;
using namespace HaSLL;

Configuration::Configuration() : Configuration(false) {}

Configuration::Configuration(bool basic)
    : logger_(LoggerManager::registerLogger("Open62541 Configuration")),
      configuration_(make_unique<UA_ServerConfig>()) {
  try {
    memset(configuration_.get(), 0, sizeof(UA_ServerConfig));
    configuration_->logging = createHaSLL();
    if (basic) {
      UA_ServerConfig_setBasics(configuration_.get());
    } else {
      UA_ServerConfig_setDefault(configuration_.get());
    }
  } catch (exception& ex) {
    string error_msg =
        "Caught exception when deserializing Configuration file: " +
        string(ex.what());
    throw Open62541_Config_Exception(error_msg);
  }
}

UA_ByteString readFile(const filesystem::path& filepath) {
  if (!filesystem::exists(filepath)) {
    throw runtime_error("File " + filepath.string() + " does not exist");
  }

  auto filesize = filesystem::file_size(filepath);
  if (filesize == 0) {
    throw runtime_error("File " + filepath.string() + " is empty");
  }

  UA_ByteString result = UA_BYTESTRING_NULL;
  UA_ByteString_allocBuffer(&result, filesize);

  ifstream file_descriptor(filepath, ios::binary);
  file_descriptor.read(reinterpret_cast<char*>(result.data), filesize);

  return result;
}

Configuration::Configuration(const string& filepath) : Configuration(true) {
  UA_ByteString json_config = readFile(filepath);

  auto status =
      UA_ServerConfig_updateFromFile(configuration_.get(), json_config);

#ifdef UA_ENABLE_HISTORIZING
  try {
    // if () { // check if database config exists
    //   throw runtime_error(
    //       "Historization enabled, but no database configuration was
    //       provided");
    // }
    // auto db_config = config.historization.value();
    // historizer_ = make_unique<Historizer>(db_config.dsn, db_config.user,
    //     db_config.auth, db_config.request_timeout,
    //     db_config.request_logging);

    // configuration_->historyDatabase.clear(&configuration_->historyDatabase);
    // configuration_->historyDatabase = historizer_->createDatabase();
  } catch (exception& ex) {
    logger_->error("Data Historization Service will not be available, due to "
                   "an exception: {}",
        ex.what());
  }
#endif // UA_ENABLE_HISTORIZING
} // namespace open62541

unique_ptr<UA_ServerConfig> Configuration::getConfig() {
  return move(configuration_);
}

#ifdef UA_ENABLE_HISTORIZING
unique_ptr<Historizer> Configuration::obtainHistorizer() {
  return move(historizer_);
}
#endif // UA_ENABLE_HISTORIZING

} // namespace open62541