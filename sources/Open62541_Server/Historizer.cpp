#include "Historizer.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Utility.hpp"

#include <chrono>
#include <cmath>
#include <date/date.h>
#include <open62541/client_subscriptions.h>
#include <open62541/server.h>
#include <string>

using namespace std;
using namespace HaSLI;
using namespace ODD;

namespace open62541 {
LoggerPtr Historizer::logger_ = LoggerPtr(); // NOLINT
DatabaseDriverPtr Historizer::db_ = DatabaseDriverPtr(); // NOLINT

Historizer::Historizer() {
  logger_ = LoggerManager::registerTypedLogger(this);
  db_ = make_unique<DatabaseDriver>();
}

Historizer::~Historizer() {
  logger_.reset();
  db_.reset();
}

template <typename... Types>
void Historizer::log(SeverityLevel level, string message, Types... args) {
  if (logger_) {
    logger_->log(level, message, args...);
  }
}

ColumnDataType getColumnDataType(const UA_DataType* variant) {
  switch (variant->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    return ColumnDataType::BOOLEAN;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    return ColumnDataType::BOOLEAN;
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
          vector<ColumnValue>{// clang-format off
              ColumnValue("Node_Id", node_id), 
              ColumnValue("Last_Updated", getCurrentTimestamp())
      }); // clang-format on
      db_->create(node_id,
          vector<Column>{// clang-format off
            Column("Index", ColumnDataType::INT, ColumnModifier::AUTO_INCREMENT),
            Column("Server_Timestamp", ColumnDataType::TIMESTAMP),
            Column("Source_Timestamp", ColumnDataType::TIMESTAMP),
            Column("Value", getColumnDataType(type))
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
  database.setEvent = nullptr;
  database.readRaw = &Historizer::readRaw;
  database.readModified = nullptr;
  database.readEvent = nullptr;
  database.readProcessed = nullptr;
  database.readAtTime = &Historizer::readAtTime;
  database.updateData = nullptr;
  database.deleteRawModified = nullptr;

  return database;
}

void Historizer::clear(UA_HistoryDatabase* /*database*/) {
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

DataType getNodeValue(UA_Variant variant) {
  switch (variant.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    auto value = *((bool*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = (intmax_t) * ((UA_SByte*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = (intmax_t) * ((UA_Int16*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = (intmax_t) * ((UA_Int32*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = (intmax_t) * ((UA_Int64*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = (uintmax_t) * ((UA_Byte*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = (uintmax_t) * ((UA_UInt16*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = (uintmax_t) * ((UA_UInt32*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = (uintmax_t) * ((UA_UInt64*)(variant.data));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    auto value = getTimestamp(*((UA_DateTime*)(variant.data)));
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    auto status_code = UA_StatusCode_name(*((UA_StatusCode*)(variant.data)));
    auto value = string(status_code);
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = (UA_Float*)(variant.data);
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = (UA_Double*)(variant.data);
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    auto byte_string = (UA_ByteString*)(variant.data);
    auto value = vector<uint8_t>(
        byte_string->data, byte_string->data + byte_string->length);
    return DataType(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    auto* ua_string = (UA_String*)(variant.data);
    auto value = string((char*)ua_string->data, ua_string->length);
    return DataType(value);
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

void Historizer::setValue(UA_Server* /*server*/, void* /*hdbContext*/,
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
            vector<ColumnValue>{// clang-format off
              ColumnValue("Source_Timestamp", source_time),
              ColumnValue("Server_Timestamp", server_time), 
              ColumnValue("Value", data)
            }); // clang-format on
        db_->update("Historized_Nodes",
            ColumnFilter(FilterType::EQUAL, "Node_Id", node_id),
            ColumnValue("Last_Updated", getCurrentTimestamp()));
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

UA_Variant toUAVariant(DataType data) {
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
        UA_String value = UA_String_fromChars(cpp_string.c_str());
        UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_delete(&value); // delete original, since a copy is already within the variant
      },
      [&result](vector<uint8_t> opaque) { 
         UA_ByteString value;
         value.length = opaque.size();
         value.data = (UA_Byte*)malloc(value.length);
         memcpy(value.data, opaque.data(), value.length);
         UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTESTRING]);
         UA_String_delete(&value); // delete original, since a copy is already within the variant
      }
      ); // clang-format on
  return result;
}

UA_DateTime toUADateTime(DataType data) {
  if (holds_alternative<string>(data)) {
    using namespace date;
    using namespace std::chrono;

    istringstream string_stream{get<string>(data)};
    system_clock::time_point time_point;
    string_stream >> parse("%F %T", time_point);

    UA_DateTimeStruct calendar_time;
    auto day_point = floor<days>(time_point);
    auto calender_date = year_month_day{day_point};
    calendar_time.year = int{calender_date.year()};
    calendar_time.month = unsigned{calender_date.month()};
    calendar_time.day = unsigned{calender_date.day()};

    auto day_time = hh_mm_ss{time_point - day_point};
    calendar_time.hour = day_time.hours().count();
    calendar_time.min = day_time.minutes().count();
    calendar_time.sec = day_time.seconds().count();
    calendar_time.milliSec = floor<milliseconds>(day_time.seconds()).count();
    calendar_time.microSec = floor<microseconds>(day_time.seconds()).count();
    calendar_time.nanoSec = floor<nanoseconds>(day_time.seconds()).count();

    return UA_DateTime_fromStruct(calendar_time);
  } else {
    throw domain_error("DataType can not be converted into UA_DateTime. "
                       "It does not contain a DateTime string.");
  }
}

UA_StatusCode expandHistoryResult(
    UA_HistoryData* result, unordered_map<size_t, vector<ColumnValue>> rows) {
  auto* data =
      (UA_DataValue*)UA_Array_new(rows.size(), &UA_TYPES[UA_TYPES_DATAVALUE]);
  if (data == nullptr) {
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

struct DatabaseNotAvailable : runtime_error {
  DatabaseNotAvailable()
      : runtime_error("Database driver is not initialized") {}
};

struct BadContinuationPoint : runtime_error {
  BadContinuationPoint() : runtime_error("Corrupted Continuation Point") {}
};

UA_ByteString* makeContinuationPoint(vector<ColumnValue> last_row) {
  if (holds_alternative<string>(last_row[0].value())) {
    auto source_timestamp =
        get<string>(last_row[0].value()); // this SHOULD be the index column
    UA_ByteString* result = UA_ByteString_new(); // should it be on heap?
    auto status = UA_ByteString_allocBuffer(result, source_timestamp.size());
    if (status == UA_STATUSCODE_GOOD) {
      memcpy(result->data, source_timestamp.c_str(), source_timestamp.size());
    } else {
      throw BadContinuationPoint();
    }
    return result;
  } else {
    throw BadContinuationPoint();
  }
}

vector<string> setColumnNames(UA_TimestampsToReturn timestampsToReturn) {
  vector<string> result;
  switch (timestampsToReturn) {
  case UA_TIMESTAMPSTORETURN_SOURCE: {
    result.emplace_back("Source_Timestamp");
    break;
  }
  case UA_TIMESTAMPSTORETURN_SERVER: {
    result.emplace_back("Server_Timestamp");
    break;
  }
  case UA_TIMESTAMPSTORETURN_BOTH: {
    result.emplace_back("Source_Timestamp");
    result.emplace_back("Server_Timestamp");
    break;
  }
  case UA_TIMESTAMPSTORETURN_NEITHER:
    [[fallthrough]];
  default: { break; }
  }
  result.emplace_back("Value");

  return result;
}

vector<ColumnFilter> setColumnFilters(
    UA_Boolean include_bounds, UA_DateTime start_time, UA_DateTime end_time) {
  vector<ColumnFilter> result;

  FilterType start_filter, end_filter;
  if (include_bounds) {
    start_filter = FilterType::GREATER_OR_EQUAL;
    end_filter = FilterType::LESS_OR_EQUAL;
  } else {
    start_filter = FilterType::GREATER;
    end_filter = FilterType::LESS;
  }

  UA_DateTime start, end;
  if (start_time < end_time) {
    start = start_time;
    end = end_time;
  } else { // set reverse order filters
    start = end_time;
    end = start_time;
  }

  if (start > 0) {
    // if start time is equal DateTime.MinValue, then there is no start filter
    result.emplace_back(start_filter, "Source_Timestamp", getTimestamp(start));
  }
  if (end > 0) {
    // if end time is equal DateTime.MinValue, then there is no end filter
    result.emplace_back(end_filter, "Source_Timestamp", getTimestamp(end));
  }

  return result;
}

vector<ColumnFilter> setColumnFilters(UA_Boolean include_bounds,
    UA_DateTime start_time, UA_DateTime end_time,
    const UA_ByteString* continuationPoint) {
  vector<ColumnFilter> result =
      setColumnFilters(include_bounds, start_time, end_time);

  if (continuationPoint != nullptr) {
    auto continuation_index =
        string((char*)continuationPoint->data, continuationPoint->length);
    result.emplace_back(FilterType::GREATER, "Index", continuation_index);
  }

  return result;
}

unordered_map<size_t, vector<ColumnValue>> Historizer::readHistory(
    const UA_ReadRawModifiedDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn, UA_NodeId node_id,
    const UA_ByteString* continuationPoint_IN,
    [[maybe_unused]] UA_ByteString* continuationPoint_OUT) { // NOLINT
  if (db_) {
    auto columns = setColumnNames(timestampsToReturn);
    auto filters = setColumnFilters(historyReadDetails->returnBounds,
        historyReadDetails->startTime, historyReadDetails->endTime,
        continuationPoint_IN);

    auto read_limit = historyReadDetails->numValuesPerNode;
    bool reverse_order = false;
    if (((historyReadDetails->startTime == 0) &&
            ((read_limit != 0) && historyReadDetails->endTime != 0)) ||
        (historyReadDetails->startTime > historyReadDetails->endTime)) {
      // if start time was not specified (start time is equal to
      // DateTime.MinValue), but end time AND read_limit is, OR end time is
      // less than start time, than the historical values should be in reverse
      // order (freshest values first, oldest ones last)
      reverse_order = true;
    }

    unordered_map<size_t, vector<ColumnValue>> results;
    if (read_limit != 0) { // if numValuesPerNode is zero, there is no limit
      OverrunPoint overrun_point;
      results = db_->select(toString(&node_id), columns, filters,
          historyReadDetails->numValuesPerNode, "Source_Timestamp",
          reverse_order, &overrun_point);
      if (overrun_point.hasMoreValues()) {
        // continuationPoint_OUT is read by the Client, not the Server
        continuationPoint_OUT = // NOLINT
            makeContinuationPoint(overrun_point.getOverrunRecord());
      }
    } else {
      results = db_->select(toString(&node_id), columns, filters, nullopt,
          "Source_Timestamp", reverse_order);
    }
    return results;
  } else {
    throw DatabaseNotAvailable();
  }
}

void Historizer::readRaw(UA_Server* /*server*/, void* /*hdbContext*/,
    const UA_NodeId* /*sessionId*/, void* /*sessionContext*/,
    const UA_RequestHeader* /*requestHeader*/,
    const UA_ReadRawModifiedDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const
        historyData) { // NOLINT parameter name set by open62541

  response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
  if (!releaseContinuationPoints) {
    for (size_t i = 0; i < nodesToReadSize; ++i) {
      try {
        auto history_values =
            readHistory(historyReadDetails, timestampsToReturn,
                nodesToRead[i].nodeId, &nodesToRead[i].continuationPoint,
                &response->results[i].continuationPoint);
        response->results[i].statusCode =
            expandHistoryResult(historyData[i], history_values);
      } catch (BadContinuationPoint& ex) {
        response->results[i].statusCode =
            UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
      } catch (DatabaseNotAvailable& ex) {
        response->responseHeader.serviceResult =
            UA_STATUSCODE_BADRESOURCEUNAVAILABLE;
      } catch (runtime_error& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
  } /* Continuation points are not stored internally, so no need to release
     * them, simply return StatusCode::Good for the entire request without
     * setting any data*/
}

void Historizer::readAtTime(UA_Server* server, void* /*hdbContext*/,
    const UA_NodeId* sessionId, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadAtTimeDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const historyData) {}

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