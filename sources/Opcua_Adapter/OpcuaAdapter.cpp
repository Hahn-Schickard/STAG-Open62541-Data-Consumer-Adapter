#include "OpcuaAdapter.hpp"

#include "LoggerRepository.hpp"

using namespace Information_Model;
using namespace Model_Event_Handler;
using namespace DCAI;
using namespace HaSLL;
using namespace std;
using namespace open62541;

OpcuaAdapter::OpcuaAdapter()
    : DataConsumerAdapterInterface("open62541 adapter"),
      server_(new Open62541Server()), node_builder_(new NodeBuilder(server_)) {}

OpcuaAdapter::~OpcuaAdapter() {
  delete server_;
  delete node_builder_;
  adapter_logger_->log(SeverityLevel::INFO, "Removing {} from logger registery",
                       adapter_logger_->getName());
  LoggerRepository::getInstance().deregisterLoger(adapter_logger_->getName());
}

void OpcuaAdapter::start() {
  if (server_->start()) {
    DCAI::DataConsumerAdapterInterface::start();
  } else {
    adapter_logger_->log(SeverityLevel::ERROR,
                         "Failled to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::stop() {
  if (server_->stop()) {
    DCAI::DataConsumerAdapterInterface::stop();
  } else {
    adapter_logger_->log(SeverityLevel::ERROR,
                         "Failled to initialize OPC UA Adapter!");
  }
}

void OpcuaAdapter::handleEvent(
    std::shared_ptr<Model_Event_Handler::NotifierEvent> event) {
  switch (event->getEventType()) {
  case NotifierEventTypeEnum::NEW_DEVICE_REGISTERED: {
    adapter_logger_->log(
        SeverityLevel::TRACE,
        "OPC UA Adapter recieved NEW_DEVICE_REGISTERED event!");
    node_builder_->addDeviceNode(event->getEvent()->device);
    break;
  }
  case NotifierEventTypeEnum::DEVICE_UPDATED: {
    adapter_logger_->log(SeverityLevel::TRACE,
                         "OPC UA Adapter recieved DEVICE_UPDATED event!");
    adapter_logger_->log(
        SeverityLevel::WARNNING,
        "Event handler for DEVICE_UPDATED event is not implemented!");
    // @TODO: Implemnent Device updates
    break;
  }
  case NotifierEventTypeEnum::DEVICE_REMOVED: {
    adapter_logger_->log(SeverityLevel::TRACE,
                         "OPC UA Adapter recieved DEVICE_REMOVED event!");
    adapter_logger_->log(
        SeverityLevel::WARNNING,
        "Event handler for DEVICE_REMOVED event is not implemented!");
    // @TODO: Implemnent Device removal
    break;
  }
  case NotifierEventTypeEnum::DEVICE_VALUE_UPDATED: {
    adapter_logger_->log(SeverityLevel::TRACE,
                         "OPC UA Adapter recieved DEVICE_VALUE_UPDATED event!");
    adapter_logger_->log(
        SeverityLevel::WARNNING,
        "Event handler for DEVICE_VALUE_UPDATED event is not implemented!");
    // @TODO: Implemnent Device value updates
    break;
  }
  default: {
    adapter_logger_->log(SeverityLevel::ERROR,
                         "OPC UA Adapter recieved an unreconized event!");
  }
  }
}