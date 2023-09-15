#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "Data_Consumer_Adapter_Interface/DataConsumerAdapterInterface.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

namespace Data_Consumer_Adapter {
class OpcuaAdapter : public DCAI {
  std::shared_ptr<open62541::Open62541Server> server_;
  std::unique_ptr<open62541::NodeBuilder> node_builder_;

  void registrate(Information_Model::NonemptyDevicePtr device) override;
  void deregistrate(const std::string& device_id) override;

public:
  OpcuaAdapter(ModelEventSourcePtr event_source);
  OpcuaAdapter(
      ModelEventSourcePtr event_source, const std::string& config_filepath);

  void start(std::vector<Information_Model::DevicePtr> devices = {}) override;
  void stop() override;
};
} // namespace Data_Consumer_Adapter
#endif //__OPCUA_ADAPTER_HPP