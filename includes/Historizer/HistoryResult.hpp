#ifndef __OPEN62541_HISTORY_RESULT_HPP
#define __OPEN62541_HISTORY_RESULT_HPP

#include <open62541/types.h>

#include <cstdint>
#include <string>
#include <vector>

namespace open62541 {
struct HistoryResult {
  int64_t index; // pqxx does not return size_t from queries, also we use -1
                 // for interpolated values
  UA_Variant value;
  std::string source_timestamp;
  std::string server_timestamp;
};

using HistoryResults = std::vector<HistoryResult>;
} // namespace open62541

#endif //__OPEN62541_HISTORY_RESULT_HPP