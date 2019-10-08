#ifndef __CONSUMER_NOTIFIER_HPP
#define __CONSUMER_NOTIFIER_HPP

#include "Device.hpp"
#include "Listener.hpp"

namespace OPCUA_Notifier {
class ConsumerNotifier : public Model_Event_Handler::Listener {
public:
  virtual void handleEvent(Model_Event_Handler::NotifierEvent *event) = 0;
};
} // namespace OPCUA_Notifier

#endif //__CONSUMER_NOTIFIER_HPP