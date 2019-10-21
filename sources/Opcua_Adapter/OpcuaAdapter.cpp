#include "OpcuaAdapter.hpp"

#include "Server_Runner.hpp"

using namespace Information_Model;
using namespace Model_Event_Handler;

void OpcuaAdapter::startOpen62541() { start(); }
void OpcuaAdapter::stopOpen62541() { stop(); }
void OpcuaAdapter::handleEvent(NotifierEvent *event) {
  switch (event->getEventType()) {
  case NotifierEventTypeEnum::NEW_DEVICE_REGISTERED: {
    addDevice(event->getEvent()->device);
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