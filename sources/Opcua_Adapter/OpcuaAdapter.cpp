#include "OpcuaAdapter.hpp"

#include "Variant_Visitor.hpp"

#include "LoggerRepository.hpp"

using namespace Information_Model;
using namespace Event_Model;
using namespace HaSLL;
using namespace std;
using namespace open62541;

namespace DCAI {
OpcuaAdapter::OpcuaAdapter(ModelEventSourcePtr event_source)
    : DataConsumerAdapterInterface(event_source, "Open62541 Adapter"),
      server_(make_shared<Open62541Server>()),
      node_builder_(make_unique<NodeBuilder>(server_)) {}

void OpcuaAdapter::start() {
  if (server_->start()) {
    DataConsumerAdapterInterface::start();
  } else {
    this->logger_->log(SeverityLevel::ERROR,
                       "Failled to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::stop() {
  if (server_->stop()) {
    DataConsumerAdapterInterface::stop();
  } else {
    this->logger_->log(SeverityLevel::ERROR,
                       "Failled to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::handleEvent(shared_ptr<ModelRegistryEvent> event) {
  if (event) {
    match(*event,
          [&](string id) {
            this->logger_->log(SeverityLevel::TRACE,
                               "OPC UA Adapter received DEVICE_REMOVED event!");
            this->logger_->log(SeverityLevel::WARNNING,
                               "Event handler for DEVICE_REMOVED event is not "
                               "implemented! Device {} will not be removed!",
                               id);
          },
          [&](shared_ptr<Device> device) {
            this->logger_->log(
                SeverityLevel::TRACE,
                "OPC UA Adapter received NEW_DEVICE_REGISTERED event!");
            node_builder_->addDeviceNode(device);
          });
  }
}
} // namespace DCAI