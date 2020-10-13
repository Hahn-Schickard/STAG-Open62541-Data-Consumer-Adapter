#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "DataConsumerAdapterInterface.hpp"
#include "Logger.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

class OpcuaAdapter : public DCAI::DataConsumerAdapterInterface {
  open62541::Open62541Server *server_;
  open62541::NodeBuilder *node_builder_;
  std::shared_ptr<HaSLL::Logger> logger_;

  void handleEvent(std::shared_ptr<DCAI::ModelRegistryEvent> event);

public:
  OpcuaAdapter(Event_Model::EventSourceInterfacePtr<DCAI::ModelRegistryEvent>
                   event_source);
  ~OpcuaAdapter();

  void start() override;
  void stop() override;
};

#endif //__OPCUA_ADAPTER_HPP