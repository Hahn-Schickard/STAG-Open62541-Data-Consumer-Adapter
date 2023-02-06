#ifndef __OPCUA_HISTORIZER_HPP
#define __OPCUA_HISTORIZER_HPP

#include "open62541/plugin/historydatabase.h"

struct Historizer {
  /**
   * @brief Create UA_MonitoredItem for a given node ide and use the data change
   * callback to as a historization call for the database
   *
   * @see
   * https://github.com/open62541/open62541/blob/master/plugins/historydata/ua_history_data_gathering_default.c#L59
   *
   */
  UA_StatusCode registerNodeId(UA_Server* server, UA_NodeId nodeId);

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
  void clear(UA_HistoryDatabase* database);

  /**
   * @brief Called when a given node receives new data value.
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param sessionId - used to identify the session the value is set for
   * @param sessionContext - used to get session context if needed
   * @param nodeId - target node id for which the new data value is set
   * @param historizing - nodes historization setting flag, if it false the
   * new value is ignored
   * @param value - new node value to historize
   */
  void setValue(UA_Server* server, void* hdbContext, const UA_NodeId* sessionId,
      void* sessionContext, const UA_NodeId* nodeId, UA_Boolean historizing,
      const UA_DataValue* value);

  /**
   * @brief Called when an event was triggered
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param originId - event origins id
   * @param emitterId - event emitters id
   * @param historicalEventFilter - historical event filter of the emitter as
   * specified by OPC UA Part 11, 5.3.2. Can be set to NULL if this property
   * does not exist
   * @param fieldList - event value
   */
  void setEvent(UA_Server* server, void* hdbContext, const UA_NodeId* originId,
      const UA_NodeId* emitterId, const UA_EventFilter* historicalEventFilter,
      UA_EventFieldList* fieldList);

  /**
   * @brief Called by UA_Server instance when a history read is requested with
   * isRawReadModified set to false
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param sessionId - used to identify the session
   * @param sessionContext - used to get session context if needed
   * @param requestHeader - ua client request header
   * @param historyReadDetails - specifies how to format the read result
   * @param timestampsToReturn - specifies which timestamps to return
   * @param releaseContinuationPoints - god fucking knows, open62541 does not
   * document it @todo: figure this out
   * @param nodesToReadSize - how many node ids to read
   * @param nodesToRead - array of node ids to read
   * @param response - used to indicate request failure
   * @param historyData - history result
   */
  void readRaw(UA_Server* server, void* hdbContext, const UA_NodeId* sessionId,
      void* sessionContext, const UA_RequestHeader* requestHeader,
      const UA_ReadRawModifiedDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const historyData);

  /**
   * @brief Called by UA_Server instance when a history read is requested with
   * isRawReadModified set to true
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param sessionId - used to identify the session
   * @param sessionContext - used to get session context if needed
   * @param requestHeader - ua client request header
   * @param historyReadDetails - specifies how to format the read result
   * @param timestampsToReturn - specifies which timestamps to return
   * @param releaseContinuationPoints - god fucking knows, open62541 does not
   * document it @todo: figure this out
   * @param nodesToReadSize - how many node ids to read
   * @param nodesToRead - array of node ids to read
   * @param response - used to indicate request failure
   * @param historyData - history result
   */
  void readModified(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadRawModifiedDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryModifiedData* const* const historyData);

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
  void readEvent(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadEventDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryEvent* const* const historyData);

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
  void readProcessed(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadProcessedDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const historyData);

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
  void readAtTime(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadAtTimeDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const historyData);

  /**
   * @brief Not documented by open62541
   *
   * Is this function even used? If so, how?
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param sessionId - used to identify the session
   * @param sessionContext - used to get session context if needed
   * @param requestHeader - ua client request header
   * @param details - data values that must be added to node id
   * @param result - history result
   */
  void updateData(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_UpdateDataDetails* details, UA_HistoryUpdateResult* result);

  /**
   * @brief @brief Not documented by open62541
   *
   * Is this function even used? If so, how?
   *
   * @param server - the parent server, that node exists on
   * @param hdbContext - not used
   * @param sessionId - used to identify the session
   * @param sessionContext - used to get session context if needed
   * @param requestHeader - ua client request header
   * @param details - node id timestamps to delete
   * @param result - history result
   */
  void deleteRawModified(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_DeleteRawModifiedDetails* details,
      UA_HistoryUpdateResult* result);

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
  void dataChanged(UA_Server* server, UA_UInt32 monitoredItemId,
      void* monitoredItemContext, const UA_NodeId* nodeId, void* nodeContext,
      UA_UInt32 attributeId, const UA_DataValue* value);
};
#endif //__OPCUA_HISTORIZER_HPP