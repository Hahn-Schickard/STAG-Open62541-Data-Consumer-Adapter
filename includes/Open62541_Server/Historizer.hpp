#ifndef __OPCUA_HISTORIZER_HPP
#define __OPCUA_HISTORIZER_HPP

#include "HaSLL/Logger.hpp"
#include "OODD/DatabaseDriver.hpp"
#include "open62541/plugin/historydatabase.h"

namespace open62541 {
struct Historizer {

  Historizer();

  ~Historizer();
  /**
   * @brief Create UA_MonitoredItem for a given node ide and use the data change
   * callback to as a historization call for the database
   *
   * @see
   * https://github.com/open62541/open62541/blob/master/plugins/historydata/ua_history_data_gathering_default.c#L59
   *
   */
  UA_StatusCode registerNodeId(
      UA_Server* server, UA_NodeId nodeId, const UA_DataType* type);

  /**
   * @brief UA_HistoryDatabase constructor
   *
   * @return UA_HistoryDatabase
   */
  UA_HistoryDatabase createDatabase();

private:
  /**
   * @brief Used as destructor for the UA_HistoryDatabase struct by UA_Server
   * instance
   *
   * @param database
   */
  static void clear(UA_HistoryDatabase* database);

  static bool isHistorized(const UA_NodeId* nodeId);

  /**
   * @brief Called when a given node receives new data value.
   *
   * @param server - not used
   * @param hdbContext - not used
   * @param sessionId - used to identify the session the value is set for
   * @param sessionContext - used to get session context if needed
   * @param nodeId - target node id for which the new data value is set
   * @param historizing - nodes historization setting flag, if it false the
   * new value is ignored
   * @param value - new node value to historize
   */
  static void setValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      UA_Boolean historizing, const UA_DataValue* value);

  /**
   * @brief Callback implementation for monitored item data change call
   *
   * @param server - the parent server, that node exists on
   * @param monitoredItemId - unkown
   * @param monitoredItemContext - not used
   * @param nodeId - node id, the new value is set for
   * @param nodeContext - not used
   * @param attributeId - unkown
   * @param value - new node value
   */
  static void dataChanged(UA_Server* server, UA_UInt32 monitoredItemId,
      void* monitoredItemContext, const UA_NodeId* nodeId, void* nodeContext,
      UA_UInt32 attributeId, const UA_DataValue* value);

  static OODD::Rows readHistory(
      const UA_ReadRawModifiedDetails* historyReadDetails,
      UA_UInt32 timeout_hint, UA_TimestampsToReturn timestampsToReturn,
      UA_NodeId node_id, const UA_ByteString* continuationPoint_IN,
      UA_ByteString* continuationPoint_OUT);

  /**
   * @brief Called by UA_Server instance when a history read is requested with
   * isRawReadModified set to false
   *
   * @param server - not used
   * @param hdbContext - not used
   * @param sessionId -  not used
   * @param sessionContext -  not used
   * @param requestHeader -  not used
   * @param historyReadDetails - specifies which values to return
   * @param timestampsToReturn - specifies how to format the result
   * @param releaseContinuationPoints - if true, returns no historic values
   * @param nodesToReadSize - how many node ids to read
   * @param nodesToRead - array of node ids to read
   * @param response - used to indicate request success/failure
   * @param historyData - history result array
   */
  static void readRaw(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadRawModifiedDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const historyData);

  static UA_StatusCode readAndAppendHistory(
      const UA_ReadAtTimeDetails* historyReadDetails, UA_UInt32 timeout_hint,
      UA_TimestampsToReturn timestampsToReturn, UA_NodeId node_id,
      const UA_ByteString* continuationPoint_IN,
      UA_ByteString* continuationPoint_OUT, UA_HistoryData* historyData);
  /**
   * @brief Not documented and not implemented by open62541
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param sessionId - used to identify the session
   * @param sessionContext - used to get session context if needed
   * @param requestHeader - ua client request header
   * @param historyReadDetails - specifies how to format the read result
   * @param timestampsToReturn - specifies which timestamps to return
   * @param releaseContinuationPoints - unknown @todo: figure this out
   * @param nodesToReadSize - how many node ids to read
   * @param nodesToRead - array of node ids to read
   * @param response - used to indicate request failure
   * @param historyData - history result
   */
  static void readAtTime(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadAtTimeDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const historyData);

  template <typename... Types>
  static void log(
      HaSLI::SeverityLevel level, std::string message, Types... args);

  static HaSLI::LoggerPtr logger_;
  static OODD::DatabaseDriverPtr db_;
};
} // namespace open62541
#endif //__OPCUA_HISTORIZER_HPP