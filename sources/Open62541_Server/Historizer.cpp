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
  auto node_id = toString(&nodeId);
  try {
    if (db_) {
      auto monitor_request = UA_MonitoredItemCreateRequest_default(nodeId);
      monitor_request.requestedParameters.samplingInterval = 100.0;
      monitor_request.monitoringMode = UA_MONITORINGMODE_REPORTING;
      auto result = UA_Server_createDataChangeMonitoredItem(server,
          UA_TIMESTAMPSTORETURN_BOTH, monitor_request, NULL,
          &Historizer::dataChanged); // save UA_UInt32 result.monitoredItemId ?
      // save nodeId and type for later checks??
      // create a table for given nodeId with UA_DataType value entries indexed
      // by source timestamp
      db_->insert("Historized_Nodes",
          vector<ODD::ColumnValue>{// clang-format off
              ODD::ColumnValue("Node_Id", node_id), 
              ODD::ColumnValue("Last_Updated", getCurrentTimestamp())
      }); // clang-format on
      db_->create(node_id,
          {// clang-format off
          ODD::Column("Server_Timestamp", ODD::ColumnDataType::TIMESTAMP),
          ODD::Column("Source_Timestamp", ODD::ColumnDataType::TIMESTAMP),
          ODD::Column("Value", getColumnDataType(type))
      }); // clang-format on
      return result.statusCode;
    } else {
      log(SeverityLevel::CRITICAL, "Database Driver is not initialized");
      return UA_STATUSCODE_BADRESOURCEUNAVAILABLE;
    }
  } catch (exception& ex) {
    log(SeverityLevel::CRITICAL,
        "An unhandled exception occurred while trying to register {} node. "
        "Exception: {}",
        node_id, ex.what());
    return UA_STATUSCODE_BADUNEXPECTEDERROR;
  }
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
        string server_time;
        if (value->hasServerTimestamp) {
          server_time = getTimestamp(value->serverTimestamp);
        }
        string source_time;
        if (value->hasSourceTimestamp) {
          source_time = getTimestamp(value->sourceTimestamp);
        }
        auto data = getNodeValue(value->value); // get data
        db_->insert(node_id,
            vector<ODD::ColumnValue>{// clang-format off
              ODD::ColumnValue("Source_Timestamp", source_time),
              ODD::ColumnValue("Server_Timestamp", server_time), 
              ODD::ColumnValue("Value", data)
            }); // clang-format on
        db_->update("Historized_Nodes",
            ODD::ColumnFilter(ODD::FilterType::EQUAL, "Node_Id", node_id),
            ODD::ColumnValue("Last_Updated", getCurrentTimestamp()));
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

UA_StatusCode appendUADataValue(UA_HistoryData* result,
    UA_DataValue* data_points, size_t data_points_size) {
  if (result->dataValuesSize == 0) {
    // no previous data to append to
    result->dataValuesSize = data_points_size;
    result->dataValues = data_points;
    return UA_STATUSCODE_GOOD;
  } else {
    UA_StatusCode status = UA_STATUSCODE_BAD; // nothing was appended
    for (size_t i = 0; i < data_points_size; ++i) {
      // there is no method in open62541 to expand an existing array with
      // another array of the same type, so we need to manually append values
      status = UA_Array_append( // use UA_Array_appendCopy instead?
          (void**)&(result->dataValues), &result->dataValuesSize,
          &(data_points[i]), &UA_TYPES[UA_TYPES_DATAVALUE]);
      if (status != UA_STATUSCODE_GOOD) {
        // early return for failure
        return status;
      }
    }
    return status;
  }
}

UA_Variant toUAVariant(ODD::DataType data) {
  UA_Variant result;
  UA_Variant_init(&result);
  match(data, // clang-format off
      [&result](bool value) { 
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BOOLEAN]); 
      },
      [&result](uintmax_t value) {
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_uintmax]); 
      },
      [&result](intmax_t value) { 
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_intmax]); 
      },
      [&result](float value) { 
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_FLOAT]);
      },
      [&result](double value) { 
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
      },
      [&result](string cpp_string) { 
        UA_String value;
        value.length = strlen(cpp_string.c_str());
        value.data = (UA_Byte*)malloc(value.length);
        memcpy(value.data, cpp_string.c_str(), value.length);
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_STRING]);
      },
      [&result](vector<uint8_t> opaque) { 
         UA_ByteString value;
         value.length = opaque.size();
         value.data = (UA_Byte*)malloc(value.length);
         memcpy(value.data, opaque.data(), value.length);
         UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTESTRING]);
      }
      ); // clang-format on
  return result;
}

UA_DateTime toUADateTime(ODD::DataType data) {
  if (holds_alternative<string>(data)) {
    using namespace date;
    using namespace std::chrono;

    istringstream string_stream{get<string>(data)};
    system_clock::time_point time_point;
    string_stream >> parse("%F %T", time_point);

    UA_DateTimeStruct calendar_time;
    auto calender_date = year_month_day{floor<days>(time_point)};
    calendar_time.year = int{calender_date.year()};
    calendar_time.month = unsigned{calender_date.month()};
    calendar_time.day = unsigned{calender_date.day()};

    auto day_time = hh_mm_ss{time_point};
    calendar_time.hour = day_time.hours().count();
    calendar_time.min = day_time.minutes().count();
    calendar_time.sec = day_time.seconds().count();
    calendar_time.milliSec = floor<milliseconds>(day_time).count();
    calendar_time.microSec = floor<microseconds>(day_time.seconds()).count();
    calendar_time.nanoSec = floor<nanoseconds>(day_time.seconds()).count();

    return UA_DateTime_fromStruct(calendar_time);
  } else {
    throw domain_error("ODD::DataType can not be converted into UA_DateTime. "
                       "It does not contain a DateTime string.");
  }
}

UA_StatusCode expandHistoryResult(UA_HistoryData* result,
    unordered_map<size_t, vector<ODD::ColumnValue>> rows) {
  auto* data =
      (UA_DataValue*)UA_Array_new(rows.size(), &UA_TYPES[UA_TYPES_DATAVALUE]);
  if (!data) {
    return UA_STATUSCODE_BADOUTOFMEMORY;
  }

  for (size_t index = 0; index < rows.size(); ++index) {
    try {
      if (rows[index].size() == 3) {
        // row has Source_Timestamp, Server_Timestamp and Value columns
        data[index].hasSourceTimestamp = true;
        data[index].hasServerTimestamp = true;
        data[index].sourceTimestamp = toUADateTime(rows[index][0].value());
        data[index].serverTimestamp = toUADateTime(rows[index][1].value());
        data[index].value = toUAVariant(rows[index][2].value());
      } else if (rows[index].size() == 2) {
        if (rows[index][0].name() == "Source_Timestamp") {
          // row has Source_Timestamp column
          data[index].hasSourceTimestamp = true;
          data[index].sourceTimestamp = toUADateTime(rows[index][0].value());
        } else {
          // row has Server_Timestamp column
          data[index].hasServerTimestamp = true;
          data[index].serverTimestamp = toUADateTime(rows[index][0].value());
        }
        // remaining row column MUST be Value
        data[index].value = toUAVariant(rows[index][1].value());
      } else {
        // row only has Value column
        data[index].hasSourceTimestamp = false;
        data[index].hasServerTimestamp = false;
        data[index].value = toUAVariant(rows[index][0].value());
      }
      data[index].hasValue = true;
    } catch (domain_error& ex) {
      // failed to decode timestamps or value for UA_DataValue, hasValue was
      // not set to true, so it MUST be false
      data[index].hasStatus = true;
      data[index].status =
          UA_STATUSCODE_BADDECODINGERROR; // mark the entire data set or just
                                          // the failed value?
    }
  }

  return appendUADataValue(result, data, rows.size());
}

void Historizer::readRaw(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadRawModifiedDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const historyData) {
  if (db_) {
    for (size_t i = 0; i < nodesToReadSize; ++i) {
      vector<string> columns;
      switch (timestampsToReturn) {
      case UA_TIMESTAMPSTORETURN_SOURCE: {
        columns.push_back("Source_Timestamp");
        break;
      }
      case UA_TIMESTAMPSTORETURN_SERVER: {
        columns.push_back("Server_Timestamp");
        break;
      }
      case UA_TIMESTAMPSTORETURN_BOTH: {
        columns.push_back("Source_Timestamp");
        columns.push_back("Server_Timestamp");
        break;
      }
      case UA_TIMESTAMPSTORETURN_NEITHER:
        [[fallthrough]];
      default: { break; }
      }
      columns.push_back("Value");

      ODD::FilterType start_filter, end_filter;
      if (historyReadDetails->returnBounds) {
        start_filter = ODD::FilterType::GREATER_OR_EQUAL;
        end_filter = ODD::FilterType::LESS_OR_EQUAL;
      } else {
        start_filter = ODD::FilterType::GREATER;
        end_filter = ODD::FilterType::LESS;
      }
      vector<ODD::ColumnFilter> filters;
      filters.emplace_back(start_filter, "Source_Timestamp",
          getTimestamp(historyReadDetails->startTime));
      filters.emplace_back(end_filter, "Source_Timestamp",
          getTimestamp(historyReadDetails->endTime));

      auto node_id = toString(&(nodesToRead[i].nodeId));
      try {
        auto history_values = db_->read(
            node_id, columns, filters, historyReadDetails->numValuesPerNode);
        // do something with releaseContinuationPoints
        // do something with &nodesToRead[i].continuationPoint
        // do something with &response->results[i].continuationPoint
        response->results[i].statusCode =
            expandHistoryResult(historyData[i], history_values);
      } catch (exception& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
    response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
  } else {
    response->responseHeader.serviceResult =
        UA_STATUSCODE_BADRESOURCEUNAVAILABLE;
  }
}

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