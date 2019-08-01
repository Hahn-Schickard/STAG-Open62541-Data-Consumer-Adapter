#ifndef __CONSUMER_NOTIFIER_HPP
#define __CONSUMER_NOTIFIER_HPP

#include "Listener.hpp"
#include "NodeInformation.h"

extern "C" void update_DataConsumer(NodeDescription *device);

namespace OPCUA_Notifier {
class ConsumerNotifier : public Notifier::Listener {
public:
  void handleEvent(Information_Model::Device *device);
};
}

#endif //__CONSUMER_NOTIFIER_HPP