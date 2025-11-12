
#ifndef __OPEN62541_STAG_DATA_CONSUMER_ADAPTER_HPP
#define __OPEN62541_STAG_DATA_CONSUMER_ADAPTER_HPP

#include <Data_Consumer_Adapter_Interface/DataConsumerAdapter.hpp>
#include <filesystem>

namespace Data_Consumer_Adapter {
DataConsumerAdapterPtr makeOpen62541Adapter(
    const DataConnector& connector, const std::filesystem::path& config);
} // namespace Data_Consumer_Adapter
#endif //__OPEN62541_STAG_DATA_CONSUMER_ADAPTER_HPP