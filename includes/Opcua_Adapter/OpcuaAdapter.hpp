#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "Data_Consumer_Adapter_Interface/DataConsumerAdapterInterface.hpp"
#include "Logger.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

namespace DCAI {
class OpcuaAdapter : public DataConsumerAdapterInterface {
  std::shared_ptr<open62541::Open62541Server> server_;
  std::unique_ptr<open62541::NodeBuilder> node_builder_;

  void handleEvent(std::shared_ptr<ModelRegistryEvent> event) override;

public:
  OpcuaAdapter(ModelEventSourcePtr event_source);

  void start() override;
  void stop() override;
};
} // namespace DCAI
#endif //__OPCUA_ADAPTER_HPP