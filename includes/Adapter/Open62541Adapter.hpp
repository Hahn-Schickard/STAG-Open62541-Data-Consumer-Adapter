
#ifndef _OPEN62541_DATA_CONSUMER_ADAPTER_HPP
#define _OPEN62541_DATA_CONSUMER_ADAPTER_HPP

#include <Data_Consumer_Adapter_Interface/DataConsumerAdapter.hpp>
#include <filesystem>

namespace Data_Consumer_Adapter {
DataConsumerAdapterPtr makeOpen62541Adapter(
    const DataConnector& connector, const std::filesystem::path& config);
} // namespace Data_Consumer_Adapter
#endif //_OPEN62541_DATA_CONSUMER_ADAPTER_HPP