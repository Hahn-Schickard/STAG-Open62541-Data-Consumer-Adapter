#ifndef __OPCUA_HISTORIZER_HPP
#define __OPCUA_HISTORIZER_HPP

#include "open62541/plugin/historydata/history_data_backend.h"
#include "open62541/plugin/historydatabase.h" // low level api, is

struct Historizer {

  UA_HistoryDataBackend crateBackend();

private:
  /**
   * UA_HistoryDataBackend destructor implementation
   */
  void deleteMembers(UA_HistoryDataBackend* backend);

  /**
   * @brief Stores given Node data value in the historization data storage
   *
   */
  UA_StatusCode setData(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      UA_Boolean historizing, const UA_DataValue* value);

  /**
   * @brief Fetches raw node data value from the historization data storage.
   *
   * Abstraction of ReadRaw low level operation.
   *
   */
  UA_StatusCode getData(UA_Server* server, const UA_NodeId* sessionId,
      void* sessionContext, const UA_HistoryDataBackend* backend,
      const UA_DateTime start, const UA_DateTime end, const UA_NodeId* nodeId,
      size_t maxSizePerResponse, UA_UInt32 numValuesPerNode,
      UA_Boolean returnBounds, UA_TimestampsToReturn timestampsToReturn,
      UA_NumericRange range, UA_Boolean releaseContinuationPoints,
      const UA_ByteString* continuationPoint,
      UA_ByteString* outContinuationPoint, UA_HistoryData* result);

  /**
   * @brief Returns the index of a value in the database based on a given
   * timestamp match criteria for a given node id.
   *
   * Low level API implementation.
   *
   * Supported match criteria:
   * @code
   * MATCH_EQUAL
   * MATCH_AFTER
   * MATCH_EQUAL_OR_AFTER
   * MATCH_BEFORE
   * MATCH_EQUAL_OR_BEFORE
   * @endcode
   */
  size_t getTimestampData(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      const UA_DateTime timestamp, const MatchStrategy strategy);

  /**
   * @brief Returns the index of the element after the last valid entry in the
   * database for a given node.
   *
   * Low level API implementation.
   *
   */
  size_t getEndIndex(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_NodeId* nodeId);

  /**
   * @brief Returns the index of the element after the last valid entry in the
   * database for a given node.
   *
   * Low level API implementation.
   *
   */
  size_t getLastIndex(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_NodeId* nodeId);

  /**
   * @brief Returns the index of the first element in the database for a node.
   *
   * Low level API implementation.
   *
   */
  size_t getFirstIndex(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_NodeId* nodeId);

  /**
   * @brief Returns the number of elements between startIndex and endIndex
   * including both.
   *
   * Low level API implementation.
   *
   */
  size_t getResultSize(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      size_t startIndex, size_t endIndex);

  /**
   * @brief Copies data values of a given range into a buffer.
   *
   * Low level API implementation.
   *
   */
  UA_StatusCode copyDataValues(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      size_t startIndex, size_t endIndex, UA_Boolean reverse, size_t valueSize,
      UA_NumericRange range, UA_Boolean releaseContinuationPoints,
      const UA_ByteString* continuationPoint,
      UA_ByteString* outContinuationPoint, size_t* providedValues,
      UA_DataValue* values);

  /**
   * @brief Returns the data value stored at a given index in the database.
   *
   * Low level API implementation.
   *
   */
  const UA_DataValue* getDataValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      size_t index);

  /**
   * @brief Returns UA_TRUE if the backend supports returning bounding
   * values for a node.
   *
   */
  UA_Boolean isBoundSupported(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_NodeId* nodeId);

  /**
   * @brief Returns UA_TRUE if the backend supports returning the requested
   * timestamps for a node.
   *
   */
  UA_Boolean isTimestampReturnSupported(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      const UA_TimestampsToReturn timestampsToReturn);

  UA_StatusCode insertDataValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      const UA_DataValue* value);

  UA_StatusCode replaceDataValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      const UA_DataValue* value);

  UA_StatusCode updateDataValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      const UA_DataValue* value);

  UA_StatusCode removeDataValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      UA_DateTime startTimestamp, UA_DateTime endTimestamp);
};

struct HistorizerLowLevelAPI {

  /**
   * @brief UA_HistoryDatabase struct destructor definition
   *
   * @param this
   */
  static void clear(UA_HistoryDatabase* this);

  /**
   * @brief Called by UA Server instance when a new node value is set.
   * UA Server uses this method to insert new data-points into the database
   *
   * @param server
   * @param hdbContext
   * @param sessionId
   * @param sessionContext
   * @param nodeId
   * @param historizing
   * @param value
   */
  static void setValue(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext, const UA_NodeId* nodeId,
      UA_Boolean historizing, const UA_DataValue* value);

  /**
   * @brief Called by UA Server instance when an event is triggered.
   * UA Server uses this method to insert new data-points into the database
   *
   * @todo: what is an event in this context?
   *
   * @param server
   * @param hdbContext
   * @param originId
   * @param emitterId
   * @param historicalEventFilter
   * @param fieldList
   */
  static void setEvent(UA_Server* server, void* hdbContext,
      const UA_NodeId* originId, const UA_NodeId* emitterId,
      const UA_EventFilter* historicalEventFilter,
      UA_EventFieldList* fieldList);

  /**
   * @brief Called by UA Server instance to get historical information from a
   * database when history read is called with isRawReadModified = false
   *
   * @param server
   * @param hdbContext
   * @param sessionId
   * @param sessionContext
   * @param requestHeader
   * @param historyReadDetails
   * @param timestampsToReturn
   * @param releaseContinuationPoints
   * @param nodesToReadSize
   * @param nodesToRead
   * @param response
   * @param historyData
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

  /**
   * @brief Called by UA Server instance to get historical information from a
   * database
   *
   * @todo: what is the difference between this and readRaw?
   *
   * @param server
   * @param hdbContext
   * @param sessionId
   * @param sessionContext
   * @param requestHeader
   * @param historyReadDetails
   * @param timestampsToReturn
   * @param releaseContinuationPoints
   * @param nodesToReadSize
   * @param nodesToRead
   * @param response
   * @param historyData
   */
  static void readModified(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadRawModifiedDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryModifiedData* const* const historyData);

  /**
   * @brief Called by UA Server instance to get historical information from a
   * database
   *
   * @todo: what is the difference between this and readRaw and readModified?
   *
   * @param server
   * @param hdbContext
   * @param sessionId
   * @param sessionContext
   * @param requestHeader
   * @param historyReadDetails
   * @param timestampsToReturn
   * @param releaseContinuationPoints
   * @param nodesToReadSize
   * @param nodesToRead
   * @param response
   * @param historyData
   */
  static void readEvent(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadEventDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryEvent* const* const historyData);

  /**
   * @brief Called by UA Server instance to get historical information from a
   * database
   *
   * @todo: what is the difference between this and readRaw, readModified and
   * readEvent?
   *
   * @param server
   * @param hdbContext
   * @param sessionId
   * @param sessionContext
   * @param requestHeader
   * @param historyReadDetails
   * @param timestampsToReturn
   * @param releaseContinuationPoints
   * @param nodesToReadSize
   * @param nodesToRead
   * @param response
   * @param historyData
   */
  static void readProcessed(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_ReadProcessedDetails* historyReadDetails,
      UA_TimestampsToReturn timestampsToReturn,
      UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
      const UA_HistoryReadValueId* nodesToRead,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const historyData);

  /**
   * @brief Called by UA Server instance to get historical information from a
   * database
   *
   * @todo: what is the difference between this and readRaw, readModified,
   * readEvent and readProcessed?
   *
   * @param server
   * @param hdbContext
   * @param sessionId
   * @param sessionContext
   * @param requestHeader
   * @param historyReadDetails
   * @param timestampsToReturn
   * @param releaseContinuationPoints
   * @param nodesToReadSize
   * @param nodesToRead
   * @param response
   * @param historyData
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

  static void updateData(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_UpdateDataDetails* details, UA_HistoryUpdateResult* result);

  static void deleteRawModified(UA_Server* server, void* hdbContext,
      const UA_NodeId* sessionId, void* sessionContext,
      const UA_RequestHeader* requestHeader,
      const UA_DeleteRawModifiedDetails* details,
      UA_HistoryUpdateResult* result);
};

#endif //__OPCUA_HISTORIZER_HPP