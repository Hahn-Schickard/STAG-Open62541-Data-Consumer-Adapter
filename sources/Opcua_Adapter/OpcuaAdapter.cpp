#include "OpcuaAdapter.hpp"

#include "Server_Runner.hpp"

using namespace Information_Model;

void OpcuaAdapter::startOpen62541() { start(); }
void OpcuaAdapter::stopOpen62541() { stop(); }
void OpcuaAdapter::handleEvent(Notifier::NotifierEvent *event) { 
    switch(event->getEventType()) {
        case Notifier::NotifierEventTypeEnum::NEW_DEVICE_REGISTERED : {
            addDevice(event->getEvent()->device.get()); 
            break;
        }
        case Notifier::NotifierEventTypeEnum::DEVICE_UPDATED :{
            // @TODO: Implemnent Device updates
            break;
        }
        case Notifier::NotifierEventTypeEnum::DEVICE_REMOVED : {
            // @TODO: Implemnent Device removal
            break;
        }
        case Notifier::NotifierEventTypeEnum::DEVICE_VALUE_UPDATED : {
            // @TODO: Implemnent Device value updates
            break;
        }
    }
}