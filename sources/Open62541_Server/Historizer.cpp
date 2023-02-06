#include "Historizer.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

#include <open62541/client_subscriptions.h>
#include <open62541/server.h>
#include <string>

using namespace std;
using namespace HaSLI;

namespace open62541 {
bool Historizer::initialized_ = false; // NOLINT

Historizer::Historizer() : logger_(LoggerManager::registerTypedLogger(this)) {}

UA_StatusCode Historizer::registerNodeId(
    UA_Server* server, UA_NodeId nodeId, const UA_DataType* type) {
  auto monitor_request = UA_MonitoredItemCreateRequest_default(nodeId);
  monitor_request.requestedParameters.samplingInterval = 100.0;
  monitor_request.monitoringMode = UA_MONITORINGMODE_REPORTING;
  auto result = UA_Server_createDataChangeMonitoredItem(server,
      UA_TIMESTAMPSTORETURN_BOTH, monitor_request, NULL,
      &Historizer::dataChanged); // save UA_UInt32 result.monitoredItemId ?
  // save nodeId and type for later checks??
  // create a table for given nodeId with UA_DataType value entries indexed by
  // source timestamp
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

void Historizer::clear(UA_HistoryDatabase* database) {
  // there is nothing to clear, since we do not use a context
}

void getNodeValue(UA_Variant data) {
  switch (data.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_GUID: {
    break;
  }
  default: {
    // log default case
  }
  }
}

string getTimestamp(UA_DateTime timestamp) {
  /* Format UA_DateTime into a %Y-%m-%d %H:%M:%S.%ms*/
  return string();
}

void Historizer::setValue(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* /*sessionId*/, void* /*sessionContext*/,
    const UA_NodeId* nodeId, UA_Boolean historizing,
    const UA_DataValue* value) {
  if (historizing) {
    if (value->hasValue) {
      string node_id = toString(nodeId);
      getNodeValue(value->value); // get data
      string server_time;
      if (value->hasServerTimestamp) {
        server_time = getTimestamp(value->serverTimestamp);
      }
      string source_time;
      if (value->hasSourceTimestamp) {
        source_time = getTimestamp(value->sourceTimestamp);
      }
      // write data change for given node to our db
    } else {
      // what to do when there is no value?
    }
  } else {
    // log not historized value warning?
  }
}

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
  UA_Boolean historize =
      attributeId & UA_ATTRIBUTEID_HISTORIZING != 0 ? UA_TRUE : UA_FALSE;

  setValue(server, NULL, sessionId, NULL, nodeId, historize, value);
}
} // namespace open62541