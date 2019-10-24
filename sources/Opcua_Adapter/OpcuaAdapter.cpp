#include "OpcuaAdapter.hpp"

using namespace Information_Model;
using namespace Model_Event_Handler;

OpcuaAdapter::OpcuaAdapter() {
  server_ = server_->getInstance();
  node_builder_ = new NodeBuilder(server_);
}

OpcuaAdapter::~OpcuaAdapter() {
  delete server_;
  delete node_builder_;
}

void OpcuaAdapter::start() { server_->start(); }

void OpcuaAdapter::stop() { server_->stop(); }

void OpcuaAdapter::handleEvent(NotifierEvent *event) {
  switch (event->getEventType()) {
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