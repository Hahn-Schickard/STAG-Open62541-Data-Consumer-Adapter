#include "OpcuaAdapter.hpp"

using namespace Information_Model;
using namespace Model_Event_Handler;
using namespace DCAI;

OpcuaAdapter::OpcuaAdapter() {
  status_       = DataConsumerAdapterStatus::UNKNOWN;
  server_       = server_->getInstance();
  node_builder_ = new NodeBuilder(server_);
}

OpcuaAdapter::~OpcuaAdapter() {
  delete server_;
  delete node_builder_;
}

void OpcuaAdapter::start() {
  status_ = DataConsumerAdapterStatus::INITIALISING;
  if(server_->start()) {
    status_ = DataConsumerAdapterStatus::RUNNING;
  }
  status_ = DataConsumerAdapterStatus::EXITED;
}

DataConsumerAdapterStatus OpcuaAdapter::getStatus() {
  return status_;
}

void OpcuaAdapter::stop() {
  if(server_->stop()) {
    status_ = DataConsumerAdapterStatus::STOPPED;
  }
}

void OpcuaAdapter::handleEvent(NotifierEvent* event) {
  switch(event->getEventType()) {
    case NotifierEventTypeEnum::NEW_DEVICE_REGISTERED: {
      node_builder_->addDeviceNode(event->getEvent()->device);
      break;
    }
    case NotifierEventTypeEnum::DEVICE_UPDATED: {
      // @TODO: Implemnent Device updates
      break;
    }
    case NotifierEventTypeEnum::DEVICE_REMOVED: {
      // @TODO: Implemnent Device removal
      break;
    }
    case NotifierEventTypeEnum::DEVICE_VALUE_UPDATED: {
      // @TODO: Implemnent Device value updates
      break;
    }
  }
}