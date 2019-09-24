#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "ConsumerNotifier.hpp"
#include "Device.hpp"

class OpcuaAdapter : public OPCUA_Notifier::ConsumerNotifier {
public:
  void startOpen62541();
  void handleEvent(Notifier::NotifierEvent *event);
  void stopOpen62541();
};

#endif //__OPCUA_ADAPTER_HPP