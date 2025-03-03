#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "Data_Consumer_Adapter_Interface/DataConsumerAdapterInterface.hpp"
#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <memory>

namespace Data_Consumer_Adapter {
struct OpcuaAdapter : public DCAI {
  using DCAI::Devices;

  OpcuaAdapter(const ModelEventSourcePtr& event_source);
  OpcuaAdapter(const ModelEventSourcePtr& event_source,
      const std::string& config_filepath);
  ~OpcuaAdapter();

  void start(const Devices& devices = {}) override;
  void stop() override;

private:
  void registrate(const Information_Model::NonemptyDevicePtr& device) override;
  void deregistrate(const std::string& device_id) override;

  std::shared_ptr<open62541::Open62541Server> server_;
  std::unique_ptr<open62541::NodeBuilder> node_builder_;
};
} // namespace Data_Consumer_Adapter
#endif //__OPCUA_ADAPTER_HPP