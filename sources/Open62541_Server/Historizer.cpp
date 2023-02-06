#include "Historizer.hpp"

#include <functional>
#include <open62541/client_subscriptions.h>

using namespace std;
using namespace std::placeholders;

UA_StatusCode Historizer::registerNodeId(
    UA_Server* server, const UA_NodeId* nodeId) {
  UA_MonitoredItemCreateRequest monitorRequest =
      UA_MonitoredItemCreateRequest_default(nodeId);
  monitorRequest.requestedParameters.samplingInterval = 100.0;
  monitorRequest.monitoringMode = UA_MONITORINGMODE_REPORTING;
  return UA_Server_createDataChangeMonitoredItem(server,
      UA_TIMESTAMPSTORETURN_BOTH, monitorRequest, NULL,
      bind(&Historizer::dataChanged, this, _1, _2, _3, _4, _5, _6, _7));
}

UA_HistoryDatabase Historizer::createDatabase() {
  UA_HistoryDatabase database;
  memset(&database, 0, sizeof(UA_HistoryDatabase));

  database.clear = &bind(&Historizer::clear, this, _1);
  // clang-format off
  database.setValue = &bind(&Historizer::setValue, this,
      _1, _2, _3, _4, _5, _6, _7);
  database.setEvent = &bind(&Historizer::setEvent, this,
      _1, _2, _3, _4, _5, _6, _7);
  database.readRaw = &bind(&Historizer::readRaw, this,
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11, _12);
  database.readModified = &bind(&Historizer::readModified, this,
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11, _12);
  database.readEvent = &bind(&Historizer::readEvent, this,
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11, _12);
  database.readProcessed = &bind(&Historizer::readProcessed, this,
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11, _12);
  database.readAtTime = &bind(&Historizer::readAtTime, this,
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11, _12);
  database.updateData = &bind(&Historizer::updateData, this,
      _1, _2, _3, _4, _5, _6, _7);
  database.deleteRawModified = &bind(&Historizer::deleteRawModified, this,
      _1, _2, _3, _4, _5, _6, _7);
  // clang-format on

  return database;
}

void Historizer::clear(UA_HistoryDatabase* database) {}

void Historizer::setValue(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_NodeId* nodeId, UA_Boolean historizing,
    const UA_DataValue* value) {}

void Historizer::setEvent(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* originId, const UA_NodeId* emitterId,
    const UA_EventFilter* historicalEventFilter, UA_EventFieldList* fieldList) {
}

void Historizer::readRaw(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadRawModifiedDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const historyData) {}

void Historizer::readModified(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadRawModifiedDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryModifiedData* const* const historyData) {}

void Historizer::readEvent(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadEventDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryEvent* const* const historyData) {}

void Historizer::readProcessed(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadProcessedDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const historyData) {}

void Historizer::readAtTime(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadAtTimeDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const historyData) {}

void Historizer::updateData(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader, const UA_UpdateDataDetails* details,
    UA_HistoryUpdateResult* result) {}

void Historizer::deleteRawModified(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_DeleteRawModifiedDetails* details,
    UA_HistoryUpdateResult* result) {}

void Historizer::dataChanged(UA_Server* server, UA_UInt32 monitoredItemId,
    void* /*monitoredItemContext*/, const UA_NodeId* nodeId,
    void* /*nodeContext*/, UA_UInt32 attributeId, const UA_DataValue* value) {
  UA_NodeId* sessionId = NULL; // obtain session id, its set to NULL in the
                               // example code, so might be imposable to do so
  UA_Boolean historize = UA_TRUE; // check if node has historization attribute
                                  // set in it's attributes
  setValue(server, NULL, sessionId, NULL, nodeId, historize, value);
}