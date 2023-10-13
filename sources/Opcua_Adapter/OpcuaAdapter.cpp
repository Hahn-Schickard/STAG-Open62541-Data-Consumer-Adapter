#include "OpcuaAdapter.hpp"

#include "HaSLL_Logger.hpp"
#include "Variant_Visitor.hpp"

using namespace Information_Model;
using namespace Event_Model;
using namespace HaSLI;
using namespace std;
using namespace open62541;

namespace Data_Consumer_Adapter {
OpcuaAdapter::OpcuaAdapter(ModelEventSourcePtr event_source)
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    : DCAI(event_source, "Open62541_Adapter") {
  registerLoggers();
  server_ = make_shared<Open62541Server>();
  node_builder_ = make_unique<NodeBuilder>(server_);
}

OpcuaAdapter::OpcuaAdapter(
    ModelEventSourcePtr event_source, const string& config_filepath)
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    : DCAI(event_source, "Open62541_Adapter") {
  registerLoggers();
  server_ = make_shared<Open62541Server>(
      make_unique<open62541::Configuration>(config_filepath));
  node_builder_ = make_unique<NodeBuilder>(server_);
}

OpcuaAdapter::~OpcuaAdapter() { removeLoggers(); }

void OpcuaAdapter::start(vector<DevicePtr> devices) {
  if (server_->start()) {
    DataConsumerAdapterInterface::start(devices);
  } else {
    logger->log(SeverityLevel::ERROR, "Failed to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::stop() {
  if (server_->stop()) {
    DataConsumerAdapterInterface::stop();
  } else {
    logger->log(SeverityLevel::ERROR, "Failed to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::registrate(NonemptyDevicePtr device) {
  logger->log(SeverityLevel::TRACE,
      "OPC UA Adapter received NEW_DEVICE_REGISTERED event for device "
      "with id",
      device->getElementId());
  node_builder_->addDeviceNode(device);
}

void OpcuaAdapter::deregistrate(const string& device_id) {
  logger->log(SeverityLevel::TRACE,
      "OPC UA Adapter received DEVICE_REMOVED event for device with id "
      "{}",
      device_id);
  node_builder_->deleteDeviceNode(device_id);
}
} // namespace Data_Consumer_Adapter