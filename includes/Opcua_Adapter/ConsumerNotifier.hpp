#ifndef __CONSUMER_NOTIFIER_HPP
#define __CONSUMER_NOTIFIER_HPP

#include "Device.hpp"
#include "Listener.hpp"

namespace OPCUA_Notifier {
class ConsumerNotifier : public Notifier::Listener {
public:
  virtual void handleEvent(Information_Model::Device *device) = 0;
};
} // namespace OPCUA_Notifier

#endif //__CONSUMER_NOTIFIER_HPP