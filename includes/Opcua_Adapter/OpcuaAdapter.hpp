#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "DataConsumerAdapterInterface.hpp"
#include "Logger.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

namespace DCAI {
class OpcuaAdapter : public DataConsumerAdapterInterface {
  std::shared_ptr<open62541::Open62541Server> server_;
  std::unique_ptr<open62541::NodeBuilder> node_builder_;
  std::shared_ptr<HaSLL::Logger> logger_;

  void handleEvent(std::shared_ptr<ModelRegistryEvent> event);

public:
  using ModelEventSourcePtr =
      std::shared_ptr<Event_Model::EventSourceInterface<ModelRegistryEvent>>;

  OpcuaAdapter(ModelEventSourcePtr event_source);
  ~OpcuaAdapter();

  void start() override;
  void stop() override;
};
} // namespace DCAI
#endif //__OPCUA_ADAPTER_HPP