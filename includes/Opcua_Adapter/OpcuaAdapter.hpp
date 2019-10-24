#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "ConsumerNotifier.hpp"
#include "Device.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

class OpcuaAdapter : public OPCUA_Notifier::ConsumerNotifier {
public:
  OpcuaAdapter();
  ~OpcuaAdapter();

  void start();
  void handleEvent(Model_Event_Handler::NotifierEvent *event);
  void stop();

private:
  NodeBuilder *node_builder_;
  Open62541Server *server_;
};

#endif //__OPCUA_ADAPTER_HPP