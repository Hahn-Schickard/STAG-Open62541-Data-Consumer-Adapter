#include "Historizer.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

#include <chrono>
#include <date/date.h>
#include <open62541/client_subscriptions.h>
#include <open62541/server.h>
#include <string>

using namespace std;
using namespace HaSLI;

namespace open62541 {
bool Historizer::initialized_ = false; // NOLINT
LoggerPtr Historizer::logger_ = LoggerPtr(); // NOLINT
ODD::DatabaseDriverPtr Historizer::db_ = ODD::DatabaseDriverPtr(); // NOLINT

Historizer::Historizer() {
  logger_ = LoggerManager::registerTypedLogger(this);
  db_ = make_unique<ODD::DatabaseDriver>();
  initialized_ = true;
}

Historizer::~Historizer() {
  logger_.reset();
  db_.reset();
  initialized_ = false;
}

template <typename... Types>
void Historizer::log(SeverityLevel level, string message, Types... args) {
  if (logger_ && initialized_) {
    logger_->log(level, message, args...);
  }
}

ODD::ColumnDataType getColumnDataType(const UA_DataType* variant) {
  switch (variant->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    return ODD::ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_GUID: {
    [[fallthrough]];
  }
  default: {
    string error_msg =
        "Unhandeled UA_DataType detected: " + string(variant->typeName);
    throw logic_error(error_msg);
  }
  }
}

string getCurrentTimestamp() {
  auto timestamp = chrono::system_clock::now();
  return date::format("%F %T", timestamp);
}

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

string getTimestamp(UA_DateTime timestamp) {
  auto calendar_time = UA_DateTime_toStruct(timestamp);
  /* Format UA_DateTime into a %Y-%m-%d %H:%M:%S.%ms%us*/
  string result = to_string(calendar_time.year) + "-" +
      to_string(calendar_time.month) + "-" + to_string(calendar_time.day) +
      " " + to_string(calendar_time.hour) + ":" + to_string(calendar_time.min) +
      ":" + to_string(calendar_time.sec) + "." +
      to_string(calendar_time.milliSec) + to_string(calendar_time.microSec);
  return result;
}

ODD::DataType getNodeValue(UA_Variant variant) {
  switch (variant.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    auto value = *((bool*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = (intmax_t) * ((UA_SByte*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = (intmax_t) * ((UA_Int16*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = (intmax_t) * ((UA_Int32*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = (intmax_t) * ((UA_Int64*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = (uintmax_t) * ((UA_Byte*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = (uintmax_t) * ((UA_UInt16*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = (uintmax_t) * ((UA_UInt32*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = (uintmax_t) * ((UA_UInt64*)(variant.data));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    auto value = getTimestamp(*((UA_DateTime*)(variant.data)));
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    auto status_code = UA_StatusCode_name(*((UA_StatusCode*)(variant.data)));
    auto value = string(status_code);
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = (UA_Float*)(variant.data);
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = (UA_Double*)(variant.data);
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    auto byte_string = (UA_ByteString*)(variant.data);
    auto value = vector<uint8_t>(
        byte_string->data, byte_string->data + byte_string->length);
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    auto* ua_string = (UA_String*)(variant.data);
    auto value = string((char*)ua_string->data, ua_string->length);
    return ODD::DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_GUID: {
    [[fallthrough]];
  }
  default: {
    string error_msg =
        "Unhandled UA_Variant type detected: " + string(variant.type->typeName);
    throw logic_error(error_msg);
  }
  }
}

void Historizer::setValue(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* /*sessionId*/, void* /*sessionContext*/,
    const UA_NodeId* nodeId, UA_Boolean historizing,
    const UA_DataValue* value) {
  string node_id = toString(nodeId);
  if (historizing) {
    if (value->hasValue) {
      try {
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
      } catch (exception& ex) {
        log(SeverityLevel::ERROR,
            "Failed to historize Node {} value due to an exception. Exception "
            "{}",
            ex.what());
      }
    } else {
      log(SeverityLevel::ERROR,
          "Failed to historize Node {} value. No data provided.", node_id);
    }
  } else {
    log(SeverityLevel::WARNING, "Node {} is not configured for historization",
        node_id);
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