#ifndef __OPEN62541_HISTORIZER_HPP
#define __OPEN62541_HISTORIZER_HPP

#include "HistorianBits.hpp"
#include "HistoryResult.hpp"

#include <HaSLL/Logger.hpp>
#include <open62541/plugin/historydatabase.h>
#include <pqxx/pqxx>

#include <filesystem>

namespace open62541 {
struct Historizer {
  Historizer();

  ~Historizer() = default;

  /**
   * @brief Create UA_MonitoredItem for a given node id and use the data change
   * callback to trigger historization call for the database
   *
   * @see
   * https://github.com/open62541/open62541/blob/master/plugins/historydata/ua_history_data_gathering_default.c#L59
   *
   */
  UA_StatusCode registerNodeId(
      UA_Server* server, UA_NodeId node_id, const UA_DataType* type);

  void write(const UA_NodeId* node_id, UA_Boolean historizing,
      const UA_DataValue* value);

  void dataChanged(const UA_NodeId* node_id, UA_UInt32 attribute_id,
      const UA_DataValue* value);

  void readRaw(const UA_RequestHeader* request_header,
      const UA_ReadRawModifiedDetails* history_read_details,
      UA_TimestampsToReturn timestamps_to_return,
      UA_Boolean release_continuation_points, size_t nodes_to_read_size,
      const UA_HistoryReadValueId* nodes_to_read,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const history_data);

  void readAtTime(const UA_RequestHeader* request_header,
      const UA_ReadAtTimeDetails* history_read_details,
      UA_TimestampsToReturn timestamps_to_return,
      UA_Boolean release_continuation_points, size_t nodes_to_read_size,
      const UA_HistoryReadValueId* nodes_to_read,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const history_data);

private:
  HistoryResults readHistory(
      const UA_ReadRawModifiedDetails* history_read_details,
      UA_UInt32 timeout_hint, UA_TimestampsToReturn timestamps_to_return,
      UA_NodeId node_id, const UA_ByteString* continuation_point_in,
      UA_ByteString* continuation_point_out);

  UA_StatusCode readAndAppendHistory(
      const UA_ReadAtTimeDetails* history_read_details, UA_UInt32 timeout_hint,
      UA_TimestampsToReturn timestamps_to_return, UA_NodeId node_id,
      const UA_ByteString* continuation_point_in,
      UA_ByteString* continuation_point_out, UA_HistoryData*);

  HaSLL::LoggerPtr logger_;
  std::unordered_map<int64_t, UA_DataTypeKind> type_map_;
};

using HistorizerPtr = std::shared_ptr<Historizer>;

/**
 * @brief UA_HistoryDatabase constructor
 *
 * @return UA_HistoryDatabase
 */
UA_HistoryDatabase createDatabaseStruct(const HistorizerPtr& historizer);
} // namespace open62541
#endif // __OPEN62541_HISTORIZER_HPP