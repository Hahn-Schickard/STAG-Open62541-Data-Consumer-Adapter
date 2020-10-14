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
    : DataConsumerAdapterInterface("open62541 adapter", event_source),
      server_(new Open62541Server()), node_builder_(new NodeBuilder(server_)),
      logger_(getLogger()) {}

OpcuaAdapter::~OpcuaAdapter() {
  delete server_;
  delete node_builder_;
  logger_->log(SeverityLevel::INFO, "Removing {} from logger registery",
               logger_->getName());
  LoggerRepository::getInstance().deregisterLoger(logger_->getName());
}

void OpcuaAdapter::start() {
  if (server_->start()) {
    DataConsumerAdapterInterface::start();
  } else {
    logger_->log(SeverityLevel::ERROR, "Failled to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::stop() {
  if (server_->stop()) {
    DataConsumerAdapterInterface::stop();
  } else {
    logger_->log(SeverityLevel::ERROR, "Failled to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::handleEvent(shared_ptr<ModelRegistryEvent> event) {
  if (event) {
    match(*event,
          [&](string id) {
            logger_->log(SeverityLevel::TRACE,
                         "OPC UA Adapter recieved DEVICE_REMOVED event!");
            logger_->log(SeverityLevel::WARNNING,
                         "Event handler for DEVICE_REMOVED event is not "
                         "implemented! Device {} will not be removed!",
                         id);
          },
          [&](shared_ptr<Device> device) {
            logger_->log(
                SeverityLevel::TRACE,
                "OPC UA Adapter recieved NEW_DEVICE_REGISTERED event!");
            node_builder_->addDeviceNode(device);
          });
  }
}
} // namespace DCAI