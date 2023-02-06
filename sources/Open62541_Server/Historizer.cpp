#include "Historizer.hpp"

#include <open62541/client_subscriptions.h>
#include <open62541/server.h>

using namespace std;

bool Historizer::initialized_ = false; // NOLINT

UA_StatusCode Historizer::registerNodeId(UA_Server* server, UA_NodeId nodeId) {
  auto monitor_request = UA_MonitoredItemCreateRequest_default(nodeId);
  monitor_request.requestedParameters.samplingInterval = 100.0;
  monitor_request.monitoringMode = UA_MONITORINGMODE_REPORTING;
  auto result = UA_Server_createDataChangeMonitoredItem(server,
      UA_TIMESTAMPSTORETURN_BOTH, monitor_request, NULL,
      &Historizer::dataChanged); // save UA_UInt32 result.monitoredItemId ?
  return result.statusCode;
}

UA_HistoryDatabase Historizer::createDatabase() {
  UA_HistoryDatabase database;
  memset(&database, 0, sizeof(UA_HistoryDatabase));

  database.clear = &Historizer::clear;
  database.setValue = &Historizer::setValue;
  database.setEvent = Historizer::setEvent;
  database.readRaw = &Historizer::readRaw;
  database.readModified = &Historizer::readModified;
  database.readEvent = &Historizer::readEvent;
  database.readProcessed = &Historizer::readProcessed;
  database.readAtTime = &Historizer::readAtTime;
  database.updateData = &Historizer::updateData;
  database.deleteRawModified = &Historizer::deleteRawModified;

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
    void* /*monitoredItemContext*/, const UA_NodeId* nodeId, void* nodeContext,
    UA_UInt32 attributeId, const UA_DataValue* value) {
  UA_NodeId* sessionId = NULL; // obtain session id, its set to NULL in the
                               // example code, so might be imposable to do so
  UA_Boolean historize = UA_TRUE; // check if node has historization attribute
                                  // set in it's attributes
  auto value_node_ctx = (UA_VariableAttributes*)nodeContext;

  setValue(server, NULL, sessionId, NULL, nodeId, historize, value);
}