#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "DataConsumerAdapterInterface.hpp"
#include "Logger.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

class OpcuaAdapter : public DCAI::DataConsumerAdapterInterface {
  Open62541Server *server_;
  NodeBuilder *node_builder_;

  void run();
  void handleEvent(std::shared_ptr<Model_Event_Handler::NotifierEvent> event);

public:
  OpcuaAdapter();
  ~OpcuaAdapter();
};

#endif //__OPCUA_ADAPTER_HPP