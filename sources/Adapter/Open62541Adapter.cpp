#include "Open62541Adapter.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

namespace Data_Consumer_Adapter {
using namespace std;
using namespace open62541;
using namespace Information_Model;

struct OpcuaAdapter : public DataConsumerAdapter {
  OpcuaAdapter(const DataConnector& connector, const filesystem::path& config)
      : DataConsumerAdapter("Open62541_Adapter", connector) {
    repo_ = make_shared<CallbackRepo>();

    auto server_config = make_unique<open62541::Configuration>(config);
#ifdef ENABLE_UA_HISTORIZING
    historizer_ = server_config->getHistorizer();
#endif // ENABLE_UA_HISTORIZING
    /* Config is consumed, so no need to save it
     * Inside UA_Server_newWithConfig assigns the config as follows
     *      server->config = *config;
     * and afterwards sets it to 0 with:
     *      memset(config, 0, sizeof(UA_ServerConfig));
     * this mimics cpp std::move()
     */
    server_ = make_shared<Open62541Server>(server_config->getConfig().get());
    builder_ = make_unique<NodeBuilder>(repo_,
#ifdef ENABLE_UA_HISTORIZING
        historizer_,
#endif // ENABLE_UA_HISTORIZING
        server_->getServer());
  }

  ~OpcuaAdapter() override = default;

  void start() override {
    if (server_->start()) {
      DataConsumerAdapter::start();
    } else {
      logger->error("Failed to start Open62541 server");
    }
  }

  void stop() override {
    if (!server_->isRunning()) {
      logger->info("Open62541 server is not running");
    }
    if (!server_->stop()) {
      logger->error("Failed to stop Open62541 server");
    }
    DataConsumerAdapter::stop();
  }

private:
  void registrate(const DevicePtr& device) override {
    logger->trace(
        "OPC UA Adapter received NEW_DEVICE_REGISTERED event for device "
        "with id",
        device->id());
    builder_->addDeviceNode(device);
  }

  void deregistrate(const string& device_id) override {
    logger->trace(
        "OPC UA Adapter received DEVICE_REMOVED event for device with id "
        "{}",
        device_id);
    builder_->deleteDeviceNode(device_id);
  }

  CallbackRepoPtr repo_;
#ifdef ENABLE_UA_HISTORIZING
  HistorizerPtr historizer_;
#endif // ENABLE_UA_HISTORIZING
  shared_ptr<Open62541Server> server_;
  unique_ptr<NodeBuilder> builder_;
};

DataConsumerAdapterPtr makeOpen62541Adapter(
    const DataConnector& connector, const filesystem::path& config) {
  return make_shared<OpcuaAdapter>(connector, config);
}
} // namespace Data_Consumer_Adapter
