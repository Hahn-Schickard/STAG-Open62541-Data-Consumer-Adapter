#include "OpcuaAdapter.hpp"

#include "LoggerRepository.hpp"

using namespace Information_Model;
using namespace Model_Event_Handler;
using namespace DCAI;
using namespace HaSLL;
using namespace std;

OpcuaAdapter::OpcuaAdapter()
    : logger_(LoggerRepository::getInstance().registerTypedLoger(this))
    , status_(DataConsumerAdapterStatus::UNKNOWN) {
  server_       = new Open62541Server();
  node_builder_ = new NodeBuilder(server_);
}

OpcuaAdapter::~OpcuaAdapter() {
  delete server_;
  delete node_builder_;
}

void OpcuaAdapter::start() {
  logger_->log(SeverityLevel::INFO, "Initialising OPC UA Adapter...");
  status_ = DataConsumerAdapterStatus::INITIALISING;
  if(server_->start()) {
    logger_->log(SeverityLevel::INFO, "OPC UA Adapter is running!");
    status_ = DataConsumerAdapterStatus::RUNNING;
  } else {
    logger_->log(SeverityLevel::ERROR, "Failled to initialize OPC UA Adapter!");
    status_ = DataConsumerAdapterStatus::EXITED;
  }
}

DataConsumerAdapterStatus OpcuaAdapter::getStatus() {
  if(status_ == DataConsumerAdapterStatus::RUNNING) {
    if(!server_->isRunning()) {
      status_ = DataConsumerAdapterStatus::EXITED;
    }
  }
  return status_;
}

void OpcuaAdapter::stop() {
  if(server_->stop()) {
    logger_->log(SeverityLevel::INFO, "OPC UA Adapter was stopped!");
    status_ = DataConsumerAdapterStatus::STOPPED;
  }
  logger_->log(SeverityLevel::ERROR, "Failled to stop OPC UA Adapter!");
}

void OpcuaAdapter::handleEvent(NotifierEvent* event) {
  switch(event->getEventType()) {
    case NotifierEventTypeEnum::NEW_DEVICE_REGISTERED: {
      logger_->log(SeverityLevel::TRACE,
          "OPC UA Adapter recieved NEW_DEVICE_REGISTERED event!");
      node_builder_->addDeviceNode(event->getEvent()->device);
      break;
    }
    case NotifierEventTypeEnum::DEVICE_UPDATED: {
      logger_->log(SeverityLevel::TRACE,
          "OPC UA Adapter recieved DEVICE_UPDATED event!");
      logger_->log(SeverityLevel::WARNNING,
          "Event handler for DEVICE_UPDATED event is not implemented!");
      // @TODO: Implemnent Device updates
      break;
    }
    case NotifierEventTypeEnum::DEVICE_REMOVED: {
      logger_->log(SeverityLevel::TRACE,
          "OPC UA Adapter recieved DEVICE_REMOVED event!");
      logger_->log(SeverityLevel::WARNNING,
          "Event handler for DEVICE_REMOVED event is not implemented!");
      // @TODO: Implemnent Device removal
      break;
    }
    case NotifierEventTypeEnum::DEVICE_VALUE_UPDATED: {
      logger_->log(SeverityLevel::TRACE,
          "OPC UA Adapter recieved DEVICE_VALUE_UPDATED event!");
      logger_->log(SeverityLevel::WARNNING,
          "Event handler for DEVICE_VALUE_UPDATED event is not implemented!");
      // @TODO: Implemnent Device value updates
      break;
    }
    default: {
      logger_->log(SeverityLevel::ERROR,
          "OPC UA Adapter recieved an unreconized event!");
    }
  }
}