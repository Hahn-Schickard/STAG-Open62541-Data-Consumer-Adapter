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
struct DatabaseNotAvailable : runtime_error {
  DatabaseNotAvailable()
      : runtime_error("Database driver is not initialized") {}
};

struct BadContinuationPoint : runtime_error {
  BadContinuationPoint() : runtime_error("Corrupted Continuation Point") {}
};

struct OutOfMemory : runtime_error {
  OutOfMemory()
      : runtime_error("There is not enough memory to complete the operation") {}
};

struct NoData : runtime_error {
  NoData() : runtime_error("No data") {}
};

struct NoBoundData : runtime_error {
  NoBoundData() : runtime_error("No bound data") {}
};

LoggerPtr Historizer::logger_ = LoggerPtr(); // NOLINT
DatabaseDriverPtr Historizer::db_ = DatabaseDriverPtr(); // NOLINT

Historizer::Historizer() {
  logger_ = LoggerManager::registerTypedLogger(this);
  db_ = make_unique<DatabaseDriver>();
  db_->create("Historized_Nodes",
      vector<Column>{// clang-format off
            Column("Node_Id", ColumnDataType::TEXT),
            Column("Last_Updated", ColumnDataType::TIMESTAMP)
      }, // clang-format on
      true);
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
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    return ColumnDataType::SMALLINT;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    return ColumnDataType::INT;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    // SQL does not have larger integer type than 64 bits
    // We will have to loose the upper most value bound
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    return ColumnDataType::BIGINT;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    return ColumnDataType::TIMESTAMP;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    return ColumnDataType::FLOAT;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    [[fallthrough]];
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    return ColumnDataType::TEXT;
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

string toSanitizedString(const UA_NodeId* nodeId) {
  return "\"" + toString(nodeId) + "\"";
}

bool Historizer::isHistorized(const UA_NodeId* nodeId) {
  auto node_id = toString(nodeId);
  auto result = db_->select(
      "Historized_Nodes", ColumnFilter(FilterType::EQUAL, "Node_Id", node_id));
  return !result.empty();
}

UA_StatusCode Historizer::registerNodeId(
    UA_Server* server, UA_NodeId nodeId, const UA_DataType* type) {
  auto node_id = toSanitizedString(&nodeId);
  try {
    if (db_) {
      if (isHistorized(&nodeId)) {
        db_->update("Historized_Nodes",
            ColumnFilter(FilterType::EQUAL, "Node_Id", toString(&nodeId)),
            ColumnValue("Last_Updated", getCurrentTimestamp()));
      } else {
        db_->insert("Historized_Nodes",
            vector<ColumnValue>{// clang-format off
              ColumnValue("Node_Id", toString(&nodeId)), // column value is automatically sanitized
              ColumnValue("Last_Updated", getCurrentTimestamp())
          }); // clang-format on
      }
      db_->create(node_id,
          vector<Column>{// clang-format off
            Column("Index", ColumnDataType::INT, ColumnModifier::AUTO_INCREMENT),
            Column("Server_Timestamp", ColumnDataType::TIMESTAMP),
            Column("Source_Timestamp", ColumnDataType::TIMESTAMP),
            Column("Value", getColumnDataType(type))
          }, // clang-format on
          true);

      auto monitor_request = UA_MonitoredItemCreateRequest_default(nodeId);
      monitor_request.requestedParameters.samplingInterval = 100.0;
      monitor_request.monitoringMode = UA_MONITORINGMODE_REPORTING;
      auto result = UA_Server_createDataChangeMonitoredItem(server,
          UA_TIMESTAMPSTORETURN_BOTH, monitor_request, NULL,
          &Historizer::dataChanged);
      return result.statusCode;
    } else {
      log(SeverityLevel::CRITICAL, "Database Driver is not initialized");
      return UA_STATUSCODE_BADDATAUNAVAILABLE;
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
  string node_id = toSanitizedString(nodeId);
  if (db_) {
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
              "Failed to historize Node {} value due to an exception. "
              "Exception: {}",
              node_id, ex.what());
        }
      } else {
        log(SeverityLevel::ERROR,
            "Failed to historize Node {} value. No data provided.", node_id);
      }
    } else {
      log(SeverityLevel::INFO, "Node {} is not configured for historization",
          node_id);
    }
  } else {
    log(SeverityLevel::CRITICAL,
        "Tried to historize Node {}, but internal database is unavailable",
        node_id);
  }
}

void Historizer::dataChanged(UA_Server* server, UA_UInt32 /*monitoredItemId*/,
    void* /*monitoredItemContext*/, const UA_NodeId* nodeId,
    void* /*nodeContext*/, UA_UInt32 attributeId, const UA_DataValue* value) {
  if (db_) {
    if (isHistorized(nodeId)) {
      UA_NodeId* session_id =
          nullptr; // obtain session id, its set to NULL in the
                   // example code, so might be imposable to do so
      UA_Boolean historize = false;
      if ((attributeId & UA_ATTRIBUTEID_HISTORIZING) != 0) {
        historize = true;
      }

      setValue(server, nullptr, session_id, nullptr, nodeId, historize, value);
    } else {
      log(SeverityLevel::WARNING,
          "Node {} has not been registered for historization. No values will "
          "be historized, until the node has finished it's registration.",
          toString(nodeId));
    }
  } else {
    log(SeverityLevel::CRITICAL,
        "Tried to historize Node {}, but internal database is unavailable",
        toString(nodeId));
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

UA_StatusCode expandHistoryResult(UA_HistoryData* result, Rows rows) {
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

/**
 * @todo: expand ContinuationPoint to store either the index of next value or
 * a future result for async select statements
 */
UA_ByteString* makeContinuationPoint(vector<ColumnValue> last_row) {
  if (holds_alternative<string>(last_row[0].value())) {
    auto source_timestamp =
        get<string>(last_row[0].value()); // this SHOULD be the index column
    UA_ByteString* result = UA_ByteString_new(); // should it be on heap?
    auto status = UA_ByteString_allocBuffer(result, source_timestamp.size());
    if (status == UA_STATUSCODE_GOOD) {
      memcpy(result->data, source_timestamp.c_str(), source_timestamp.size());
    } else {
      throw OutOfMemory();
    }
    return result;
  } else {
    throw BadContinuationPoint();
  }
}

Rows Historizer::readHistory(
    const UA_ReadRawModifiedDetails* historyReadDetails,
    UA_UInt32 /*timeout_hint*/, UA_TimestampsToReturn timestampsToReturn,
    UA_NodeId node_id, const UA_ByteString* continuationPoint_IN,
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

    Rows results;
    if (read_limit != 0) { // if numValuesPerNode is zero, there is no limit
      OverrunPoint overrun_point;
      /**
       * @todo: use an async select request or a batch read, and check if
       * timeout_hint elapsed, if it did set a continuation point
       *
       */
      results = db_->select(toString(&node_id), columns, filters,
          historyReadDetails->numValuesPerNode, "Source_Timestamp",
          reverse_order, &overrun_point);
      if (overrun_point.hasMoreValues()) {
        // continuationPoint_OUT is read by the Client, not the Server
        continuationPoint_OUT = // NOLINT
            makeContinuationPoint(overrun_point.getOverrunRecord());
      }
    } else {
      /**
       * @todo: use an async select request or a batch read, and check if
       * timeout_hint elapsed, if it did set a continuation point
       *
       */
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
    const UA_RequestHeader* requestHeader,
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
        auto history_values = readHistory(historyReadDetails,
            requestHeader->timeoutHint, timestampsToReturn,
            nodesToRead[i].nodeId, &nodesToRead[i].continuationPoint,
            &response->results[i].continuationPoint);
        response->results[i].statusCode =
            expandHistoryResult(historyData[i], history_values);
      } catch (BadContinuationPoint& ex) {
        response->results[i].statusCode =
            UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
      } catch (DatabaseNotAvailable& ex) {
        response->responseHeader.serviceResult =
            UA_STATUSCODE_BADDATAUNAVAILABLE;
      } catch (OutOfMemory& ex) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
      } catch (runtime_error& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
  } /**
     * @todo: release response->results[i]->historyData ?
     * as specified in:
     * https://github.com/open62541/open62541/blob/master/include/open62541/plugin/historydatabase.h#L73
     */
  /* Continuation points are not stored internally, so no need to release
   * them, simply return StatusCode::Good for the entire request without
   * setting any data*/
}

DataType operator*(const DataType& lhs, const intmax_t& rhs) {
  DataType result;
  match(lhs,
      [&](bool /*value*/) {
        throw logic_error("Can not multiply Boolean value");
      },
      [&](uintmax_t value) { result = value * abs(rhs); },
      [&](intmax_t value) { result = value * rhs; },
      [&](float value) { result = value * rhs; },
      [&](double value) { result = value * rhs; },
      [&](string /*value*/) { throw logic_error("Can not multiply Text"); },
      [&](vector<uint8_t> /*value*/) {
        throw logic_error("Can not multiply Opaque data");
      });
  return result;
}

DataType operator+(const DataType& lhs, const DataType& rhs) {
  DataType result;
  match(lhs,
      [&](bool /*value*/) {
        throw invalid_argument("Can not add to a boolean");
      },
      [&](uintmax_t value) {
        if (holds_alternative<uintmax_t>(rhs)) {
          auto addition = get<uintmax_t>(rhs);
          result = value + addition;
        } else {
          throw invalid_argument(
              "Can not add non unsigned int value to a unsigned int");
        }
      },
      [&](intmax_t value) {
        if (holds_alternative<intmax_t>(rhs)) {
          auto addition = get<intmax_t>(rhs);
          result = value + addition;
        } else {
          throw invalid_argument(
              "Can not add non signed int value to a signed int");
        }
      },
      [&](float value) {
        if (holds_alternative<float>(rhs)) {
          auto addition = get<float>(rhs);
          result = value + addition;
        } else {
          throw invalid_argument("Can not add non float value to a float");
        }
      },
      [&](double value) {
        if (holds_alternative<double>(rhs)) {
          auto addition = get<double>(rhs);
          result = value + addition;
        } else {
          throw invalid_argument("Can not add non double value to a double");
        }
      },
      [&](string /*value*/) { throw logic_error("Can not add to Text"); },
      [&](vector<uint8_t> /*value*/) {
        throw logic_error("Can not add to Opaque data");
      });
  return result;
}

DataType operator-(const DataType& lhs, const DataType& rhs) {
  DataType result;
  match(lhs,
      [&](bool /*value*/) {
        throw invalid_argument("Can not subtract from a boolean");
      },
      [&](uintmax_t value) {
        if (holds_alternative<uintmax_t>(rhs)) {
          auto addition = get<uintmax_t>(rhs);
          result = value - addition;
        } else {
          throw invalid_argument(
              "Can not subtract non unsigned int value from a unsigned int");
        }
      },
      [&](intmax_t value) {
        if (holds_alternative<intmax_t>(rhs)) {
          auto addition = get<intmax_t>(rhs);
          result = value - addition;
        } else {
          throw invalid_argument(
              "Can not subtract non signed int value from a signed int");
        }
      },
      [&](float value) {
        if (holds_alternative<float>(rhs)) {
          auto addition = get<float>(rhs);
          result = value - addition;
        } else {
          throw invalid_argument(
              "Can not subtract non float value from a float");
        }
      },
      [&](double value) {
        if (holds_alternative<double>(rhs)) {
          auto addition = get<double>(rhs);
          result = value - addition;
        } else {
          throw invalid_argument(
              "Can not subtract non double value from a double");
        }
      },
      [&](string /*value*/) {
        throw logic_error("Can not subtract from Text");
      },
      [&](vector<uint8_t> /*value*/) {
        throw logic_error("Can not subtract from Opaque data");
      });
  return result;
}

DataType operator/(const DataType& lhs, const intmax_t& rhs) {
  DataType result;
  match(lhs,
      [&](bool /*value*/) {
        throw logic_error("Can not divide Boolean value");
      },
      [&](uintmax_t value) { result = value / rhs; },
      [&](intmax_t value) { result = value / rhs; },
      [&](float value) { result = value / rhs; },
      [&](double value) { result = value / rhs; },
      [&](string /*value*/) { throw logic_error("Can not divide Text"); },
      [&](vector<uint8_t> /*value*/) {
        throw logic_error("Can not divide Opaque data");
      });
  return result;
}

vector<ColumnValue> interpolateValues(
    UA_DateTime target_timestamp, bool simple_bounds, Rows first, Rows second) {
  vector<ColumnValue> result;
  // both maps MUST have 1 row each
  if (first.size() != 1 || second.size() != 1) {
    throw logic_error("Multiple nearest match values are not supported");
  }

  auto first_row = first.begin()->second;
  auto second_row = second.begin()->second;
  DataType first_nearest_value, second_nearest_value;
  string data_point_name;
  // set result timestamp columns and nearest_value DataTypes
  switch (first_row.size()) { // MUST either be equal to 1,2 or 3
  case 1: {
    first_nearest_value = first_row[0].value();
    second_nearest_value = second_row[0].value();
    data_point_name = first_row[0].name();
    break;
  }
  case 2: {
    result.emplace_back(first_row[0].name(), getTimestamp(target_timestamp));
    first_nearest_value = first_row[1].value();
    second_nearest_value = second_row[1].value();
    data_point_name = first_row[1].name();
    break;
  }
  case 3: {
    result.emplace_back(first_row[0].name(), getTimestamp(target_timestamp));
    result.emplace_back(first_row[1].name(), getTimestamp(target_timestamp));
    first_nearest_value = first_row[2].value();
    second_nearest_value = second_row[2].value();
    data_point_name = first_row[2].name();
    break;
  }
  default: {
    throw NoBoundData();
    break;
  }
  }

  auto first_nearest_timestamp = toUADateTime(first_row[0].value());
  auto second_nearest_timestamp = toUADateTime(second_row[0].value());
  DataType interpolated_data;
  if (simple_bounds) {
    // use linear interpolation between the nearest bounding values
    intmax_t weight1 = second_nearest_timestamp - target_timestamp;
    intmax_t weight2 = target_timestamp - first_nearest_timestamp;
    intmax_t denominator = second_nearest_timestamp - first_nearest_timestamp;
    interpolated_data =
        ((first_nearest_value * weight1) + (second_nearest_value * weight2)) /
        denominator;
  } else {
    // use formula from OPC UA Part 13 Aggregates specification section 3.1.8
    intmax_t weight = target_timestamp - first_nearest_timestamp;
    auto value_diff = second_nearest_value - first_nearest_value;
    intmax_t denominator = second_nearest_timestamp - first_nearest_timestamp;
    interpolated_data =
        ((value_diff * weight) / denominator) + first_nearest_value;
  }
  result.emplace_back(ColumnValue(data_point_name, interpolated_data));
  return result;
}

/**
 * @brief Defines Aggregate Result Status Code expansion
 *
 * Defined in UA Part 4: Services Table 181 DataValue InfoBits
 *
 */
namespace HistorianBits {
enum class DataLocation {
  RAW = 0x00,
  CALCULATED = 0x01,
  INTERPOLATED = 0x02,
  RESERVED = 0x03
};

void setHistorianBits(UA_StatusCode* status, DataLocation data_loc,
    bool is_partial = false, bool has_extra = false,
    bool has_multiple = false) {
  if (UA_StatusCode_isBad(*status)) {
    string error_msg =
        "Can not set historian bits for " + string(UA_StatusCode_name(*status));
    throw invalid_argument(error_msg);
  }
  if (data_loc == DataLocation::RESERVED) {
    throw invalid_argument(
        "Data location type can not be set to reserved type.");
  }
  *status |= static_cast<uint32_t>(data_loc);

  static constexpr uint8_t PARTIAL_DATA_OFFSET = 2;
  static constexpr uint8_t EXTRA_DATA_OFFSET = 3;
  static constexpr uint8_t MULTI_VALUE_OFFSET = 4;

  *status |= (static_cast<uint32_t>(is_partial) << PARTIAL_DATA_OFFSET) |
      (static_cast<uint32_t>(has_extra) << EXTRA_DATA_OFFSET) |
      (static_cast<uint32_t>(has_multiple) << MULTI_VALUE_OFFSET);
}

UA_StatusCode setHistorianBits(const UA_StatusCode* status,
    DataLocation data_loc, bool is_partial = false, bool has_extra = false,
    bool has_multiple = false) {
  UA_StatusCode result = *status; // obtain a copy of original status code
  setHistorianBits(&result, data_loc, is_partial, has_extra, has_multiple);
  return result;
}

DataLocation getDataLocation(const UA_StatusCode status) {
  static constexpr uint32_t HISTORIAN_BITS_MASK = 0x03;

  return static_cast<DataLocation>(status & HISTORIAN_BITS_MASK);
}

string toString(DataLocation location) {
  switch (location) {
  case DataLocation::RAW: {
    return "Raw";
  }
  case DataLocation::CALCULATED: {
    return "Calculated";
  }
  case DataLocation::INTERPOLATED: {
    return "Interpolated";
  }
  default: { return "Reserved"; }
  }
}

/**
 * @brief Checks if StatusCode indicates that data value is calculated with
 * an incomplete interval
 *
 * @param status
 * @return UA_Boolean
 */
UA_Boolean hasPartialValue(const UA_StatusCode status) {
  static constexpr uint32_t PARTIAL_DATA_MASK = 0x1B;

  return (status & PARTIAL_DATA_MASK) > 0;
}

/**
 * @brief Checks if StatusCode indicates that the Raw data value supersedes
 * other data at the same timestamp
 *
 * @param status
 * @return UA_Boolean
 */
UA_Boolean hasExtraData(const UA_StatusCode status) {
  static constexpr uint32_t EXTRA_DATA_MASK = 0x17;

  return (status & EXTRA_DATA_MASK) > 0;
}

/**
 * @brief Checks if StatusCode indicates that the multiple data values match
 * the Aggregate criteria
 *
 * @param status
 * @return UA_Boolean
 */
UA_Boolean hasMultipleValues(const UA_StatusCode status) {
  static constexpr uint32_t MULTI_VALUE_MASK = 0x0F;

  return (status & MULTI_VALUE_MASK) > 0;
}
} // namespace HistorianBits

UA_StatusCode Historizer::readAndAppendHistory(
    const UA_ReadAtTimeDetails* historyReadDetails, UA_UInt32 /*timeout_hint*/,
    UA_TimestampsToReturn timestampsToReturn, UA_NodeId node_id,
    const UA_ByteString* /*continuationPoint_IN*/,
    [[maybe_unused]] UA_ByteString* continuationPoint_OUT,
    UA_HistoryData* historyData) { // NOLINT
  if (db_) {
    auto columns = setColumnNames(timestampsToReturn);

    UA_StatusCode status = UA_STATUSCODE_GOOD;
    using namespace HistorianBits;
    Rows results;
    for (size_t i = 0; i < historyReadDetails->reqTimesSize; ++i) {
      auto timestamp = getTimestamp(historyReadDetails->reqTimes[i]);
      /**
       * @todo: use an async select request or a batch read, and check if
       * timeout_hint elapsed, if it did set a continuation point
       *
       */
      auto timestamp_results = db_->select(toString(&node_id), columns,
          ColumnFilter(FilterType::EQUAL, timestamp), nullopt,
          "Source_Timestamp");

      if (timestamp_results.empty()) {
        /**
         * @todo: use an async select requests or a batch reads, and check if
         * timeout_hint elapsed for both nearest_before_result and
         * nearest_after_result select requests, if it did set a continuation
         * point for next
         *
         */
        auto nearest_before_result = db_->select(toString(&node_id), columns,
            ColumnFilter(FilterType::LESS, timestamp), 1, "Source_Timestamp",
            true);
        auto nearest_after_result = db_->select(toString(&node_id), columns,
            ColumnFilter(FilterType::GREATER, timestamp), 1, "Source_Timestamp",
            false);
        // indexes are only used to iterate over the results map, so we only
        // need to make sure that they are all unique, since bounding values
        // by defintion do not meet our aggregate criteria, their indexes will
        // never be in the results maps, thus it is safe to use their indexes
        auto index = nearest_after_result.begin()->first;
        results.emplace(index,
            // useSimpleBounds=False dictates that we need to find first
            // Non-Bad Raw values to use for interpolation, however it is not
            // specified what is a Non-Bad Raw value, so we assume that the
            // nearest RAW values to our target timestamp qualify as Non-Bad
            interpolateValues(historyReadDetails->reqTimes[i],
                historyReadDetails->useSimpleBounds, nearest_before_result,
                nearest_after_result));
        setHistorianBits(&status, DataLocation::INTERPOLATED);
      } else {
        results.merge(timestamp_results);
        setHistorianBits(&status, DataLocation::RAW);
      }
    }
    expandHistoryResult(historyData, results);
    return status;
  } else {
    throw DatabaseNotAvailable();
  }
}

void Historizer::readAtTime(UA_Server* /*server*/, void* /*hdbContext*/,
    const UA_NodeId* /*sessionId*/, void* /*sessionContext*/,
    const UA_RequestHeader* requestHeader,
    const UA_ReadAtTimeDetails* historyReadDetails,
    UA_TimestampsToReturn timestampsToReturn,
    UA_Boolean releaseContinuationPoints, size_t nodesToReadSize,
    const UA_HistoryReadValueId* nodesToRead, UA_HistoryReadResponse* response,
    UA_HistoryData* const* const
        historyData) { // NOLINT parameter name set by open62541
  response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
  if (!releaseContinuationPoints) {
    for (size_t i = 0; i < nodesToReadSize; ++i) {
      try {
        response->results[i].statusCode = readAndAppendHistory(
            historyReadDetails, requestHeader->timeoutHint, timestampsToReturn,
            nodesToRead[i].nodeId, &nodesToRead[i].continuationPoint,
            &response->results[i].continuationPoint, historyData[i]);
      } catch (logic_error& ex) {
        // Target Node Id values can not be interpolated due to mismatching
        // data types or non aggregable data types (text, opaque values,
        // etc...) OR resulting timestamps where malformed and could not be
        // decoded as UA_DateTime
        response->results[i].statusCode =
            UA_STATUSCODE_BADAGGREGATEINVALIDINPUTS;
      } catch (NoBoundData& ex) {
        response->results[i].statusCode = UA_STATUSCODE_BADBOUNDNOTFOUND;
      } catch (NoData& ex) {
        response->results[i].statusCode = UA_STATUSCODE_BADNODATA;
      } catch (BadContinuationPoint& ex) {
        response->results[i].statusCode =
            UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
      } catch (DatabaseNotAvailable& ex) {
        response->responseHeader.serviceResult =
            UA_STATUSCODE_BADDATAUNAVAILABLE;
      } catch (OutOfMemory& ex) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
      } catch (runtime_error& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
  }
}
} // namespace open62541