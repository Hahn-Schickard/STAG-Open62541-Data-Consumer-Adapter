#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "DataConsumerAdapterInterface.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

class OpcuaAdapter : public DCAI::DataConsumerAdapterInterface {
 public:
  OpcuaAdapter();
  ~OpcuaAdapter();

  void start();
  DCAI::DataConsumerAdapterStatus getStatus();
  void handleEvent(Model_Event_Handler::NotifierEvent* event);
  void stop();

 private:
  NodeBuilder* node_builder_;
  Open62541Server* server_;
  DCAI::DataConsumerAdapterStatus status_;
};

#endif   //__OPCUA_ADAPTER_HPP