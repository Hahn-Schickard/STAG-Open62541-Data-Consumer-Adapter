#include "OpcuaAdapter.hpp"

namespace Data_Consumer_Adapter {
using namespace Information_Model;
using namespace std;
using namespace open62541;

OpcuaAdapter::OpcuaAdapter(const DataConnector& connector)
    : DataConsumerAdapter("Open62541_Adapter", connector) {
  server_ = make_shared<Open62541Server>();
  node_builder_ = make_unique<NodeBuilder>(server_);
}

OpcuaAdapter::OpcuaAdapter(
    const DataConnector& connector, const string& config_filepath)
    : DataConsumerAdapter("Open62541_Adapter", connector) {
  server_ = make_shared<Open62541Server>(
      make_unique<open62541::Configuration>(config_filepath));
  node_builder_ = make_unique<NodeBuilder>(server_);
}

void OpcuaAdapter::start() {
  if (server_->start()) {
    DataConsumerAdapter::start();
  } else {
    logger->error("Failed to start Open62541 server");
  }
}

void OpcuaAdapter::stop() {
  if (!server_->isRunning()) {
    logger->info("Open62541 server is not running");
  }
  if (!server_->stop()) {
    logger->error("Failed to stop Open62541 server");
  }
  DataConsumerAdapter::stop();
}

void OpcuaAdapter::registrate(const DevicePtr& device) {
  logger->trace(
      "OPC UA Adapter received NEW_DEVICE_REGISTERED event for device "
      "with id",
      device->id());
  node_builder_->addDeviceNode(device);
}

void OpcuaAdapter::deregistrate(const string& device_id) {
  logger->trace(
      "OPC UA Adapter received DEVICE_REMOVED event for device with id "
      "{}",
      device_id);
  node_builder_->deleteDeviceNode(device_id);
}
} // namespace Data_Consumer_Adapter