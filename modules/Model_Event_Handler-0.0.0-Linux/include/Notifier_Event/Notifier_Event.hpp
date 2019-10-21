#ifndef __NOTIFIER_EVENT_HPP
#define __NOTIFIER_EVENT_HPP

#include "Device.hpp"
#include <exception>
#include <memory>
#include <new>
#include <stdexcept>
#include <string.h>
#include <string>

namespace Model_Event_Handler {
typedef struct DeviceValueStruct {
  std::string deviceId;
  // TODO: handle values buffer
} DeviceValue;

typedef union EventUnionType {
  std::shared_ptr<Information_Model::Device> device;
  std::string device_ID;
  std::shared_ptr<DeviceValue> device_value;

  EventUnionType() { memset(this, 0, sizeof(*this)); }

  ~EventUnionType() {}
} EventUnion;

typedef enum NotifierEventTypeEnum {
  NEW_DEVICE_REGISTERED,
  DEVICE_UPDATED,
  DEVICE_REMOVED,
  DEVICE_VALUE_UPDATED
} NotifierEventType;

class NotifierEvent {
public:
  NotifierEvent(NotifierEventType event_type,
                std::shared_ptr<Information_Model::Device> new_device) {
    if (event_type == NotifierEventType::NEW_DEVICE_REGISTERED ||
        event_type == NotifierEventType::DEVICE_UPDATED) {
      type = event_type;
      event = new EventUnion();
      event->device = new_device;
    } else {
      //@TODO: Log malformated notifier event
      throw std::runtime_error(
          "This function is for device initialization/update only.");
    }
  }

  NotifierEvent(NotifierEventType event_type, std::string device_ID) {
    if (event_type == NotifierEventType::DEVICE_REMOVED) {
      type = event_type;
      event = new EventUnion();
      event->device_ID = device_ID;
    } else {
      //@TODO: Log malformated notifier event
      throw std::runtime_error("This function is for device removal only.");
    }
  }

  NotifierEvent(NotifierEventType event_type,
                std::shared_ptr<DeviceValue> device_value) {
    if (event_type == NotifierEventType::DEVICE_VALUE_UPDATED) {
      type = event_type;
      event = new EventUnion();
      event->device_value = device_value;
    } else {
      //@TODO: Log malformated notifier event
      throw std::runtime_error(
          "This function is for device value notifications only.");
    }
  }

  ~NotifierEvent() { delete event; }

  NotifierEventType getEventType() { return type; }

  EventUnion *getEvent() { return event; }

private:
  NotifierEventType type;
  EventUnion *event;
};
} // namespace Model_Event_Handler

#endif //__NOTIFIER_EVENT_HPP