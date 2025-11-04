#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "NodeBuilder.hpp"
#include "Open62541Server.hpp"

#include <Data_Consumer_Adapter_Interface/DataConsumerAdapter.hpp>

#include <filesystem>
#include <memory>

namespace Data_Consumer_Adapter {
struct OpcuaAdapter : public DataConsumerAdapter {
  using DataConsumerAdapter::Devices;

  OpcuaAdapter(const DataConnector& connector);
  OpcuaAdapter(
      const DataConnector& connector, const std::filesystem::path& config);
  ~OpcuaAdapter() override = default;

  void start() override;
  void stop() override;

private:
  void registrate(const Information_Model::DevicePtr& device) override;
  void deregistrate(const std::string& device_id) override;

  std::shared_ptr<open62541::Open62541Server> server_;
  std::unique_ptr<open62541::NodeBuilder> node_builder_;
};
} // namespace Data_Consumer_Adapter
#endif //__OPCUA_ADAPTER_HPP