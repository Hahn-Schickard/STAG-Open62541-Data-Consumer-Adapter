#ifdef UA_ENABLE_HISTORIZING
#include "Historizer.hpp"
#include "Utility.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <date/date.h>
#include <open62541/client_subscriptions.h>
#include <open62541/server.h>

#include <chrono>
#include <cmath>

#include <string>

namespace open62541 {
using namespace std;
using namespace HaSLL;
using namespace pqxx;

struct ConnectionUnavailable : runtime_error {
  ConnectionUnavailable()
      : runtime_error("Connection information does not exist") {}
};

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

string readConfig(const filesystem::path& config) {
  // @todo: read configuration file to get database backend, authentication and
  // table info
  return "";
}

void createDomainRestrictions(work* transaction) {
  // NOLINTBEGIN(readability-magic-numbers)
  unordered_map<string, size_t> domains{
      // clang-format off
      {"OPCUA_TINYUINT", 256},
      {"OPCUA_SMALLUINT", 65536},
      {"OPCUA_UINT", 4294967296},
      {"OPCUA_STATUSCODE", 4294967296} // status code uses 32 bit uint
  }; // clang-format on
  // NOLINTEND(readability-magic-numbers)
  string create = "CREATE DOMAIN IF NOT EXISTS ";
  for (const auto& [domain, limit] : domains) {
    transaction->exec(
        create + "$1 AS INTEGER CHECK (VALUE >= 0 AND VALUE < $2)",
        {domain, limit});
    transaction->commit();
  }
  // int8 requires different limits, hence we create it separately
  transaction->exec(create +
      "OPCUA_TINYINT AS INTEGER CHECK (VALUE >= -128 AND VALUE < 128)");
  transaction->commit();

  // uint64 does not fit into integer, hence we use numeric
  transaction->exec(create +
      "OPCUA_BIGUINT AS NUMERIC CHECK (VALUE >= 0 AND VALUE "
      "< 18446744073709551616)");
  transaction->commit();
}

unordered_map<int64_t, UA_DataTypeKind> queryTypeOIDs(work* transaction) {
  unordered_map<string, size_t> typenames{// clang-format off
      {"BOOLEAN", UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN},
      {"OPCUA_TINYUINT", UA_DataTypeKind::UA_DATATYPEKIND_BYTE},
      {"OPCUA_TINYINT", UA_DataTypeKind::UA_DATATYPEKIND_SBYTE},
      {"OPCUA_SMALLUINT", UA_DataTypeKind::UA_DATATYPEKIND_UINT16},
      {"SMALLINT", UA_DataTypeKind::UA_DATATYPEKIND_INT16},
      {"OPCUA_UINT", UA_DataTypeKind::UA_DATATYPEKIND_UINT32},
      {"INT", UA_DataTypeKind::UA_DATATYPEKIND_INT32},
      {"OPCUA_BIGUINT", UA_DataTypeKind::UA_DATATYPEKIND_UINT64},
      {"BIGINT", UA_DataTypeKind::UA_DATATYPEKIND_INT64},
      {"OPCUA_STATUSCODE", UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE},
      {"REAL", UA_DataTypeKind::UA_DATATYPEKIND_FLOAT},
      {"DOUBLE PRECISION", UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE},
      {"TIMESTAMP", UA_DataTypeKind::UA_DATATYPEKIND_DATETIME},
      {"BYTEA", UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING},
      {"TEXT", UA_DataTypeKind::UA_DATATYPEKIND_STRING}
  }; // clang-format on
  unordered_map<int64_t, UA_DataTypeKind> result;
  for (const auto& [type_name, type_code] : typenames) {
    auto oid = transaction
                   ->exec("SELECT oid FROM pg_type WHERE typname = $1",
                       params{type_name})
                   .expect_rows(1)
                   .expect_columns(1)
                   .at(0)
                   .at(0)
                   .as<long>();
    result.emplace(oid, static_cast<UA_DataTypeKind>(type_code));
  }
  return result;
}

Historizer::Historizer(const filesystem::path& config) {
  if (!logger_) {
    logger_ = LoggerManager::registerTypedLogger(this);
  }

  if (!connection_info_.empty()) {
    connection_info_ = readConfig(config);
  }

  auto session = connect();
  work transaction(session);
  transaction.exec("CREATE TABLE IF NOT EXISTS Historized_Nodes("
                   "Node_ID TEXT PRIMARY KEY NOT NULL, "
                   "Last_Updated TIMESTAMP(6) NOT NULL"
                   ");");
  transaction.commit();
  createDomainRestrictions(&transaction);
  type_map_ = queryTypeOIDs(&transaction);
}

Historizer::~Historizer() { logger_.reset(); }

template <typename... Types>
void Historizer::log(
    SeverityLevel level, const string& message, const Types&... args) {
  if (logger_) {
    logger_->log(level, message, args...);
  }
}

pqxx::connection Historizer::connect() {
  if (!connection_info_.empty()) {
    return connection(connection_info_);
  } else {
    throw ConnectionUnavailable();
  }
}

bool Historizer::checkTable(const string& name) {
  auto session = connect();
  work transaction(session);
  auto result = transaction.exec(
      "SELECT 1 FROM information_schema.tables WHERE table_name = " + name);
  return !result.empty();
}

string toPostgreDataType(const UA_DataType* variant) {
  switch (variant->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    return "BOOLEAN";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    return "OPCUA_TINYUINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    return "OPCUA_TINYINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    return "OPCUA_SMALLUINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    return "SMALLINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    return "OPCUA_UINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    return "INT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    return "OPCUA_BIGUINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    return "BIGINT";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    return "OPCUA_STATUSCODE";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    return "REAL";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    return "DOUBLE PRECISION";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    return "TIMESTAMP(6)";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    return "BYTEA";
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    return "TEXT";
  }
  // NOLINTNEXTLINE(bugprone-branch-clone)
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

string toSanitizedString(const UA_NodeId* node_id) {
  return "\"" + toString(node_id) + "\"";
}

bool Historizer::isHistorized(const UA_NodeId* node_id) {
  auto session = connect();
  work transaction(session);
  auto result = transaction.exec(
      "SELECT 1 FROM Historized_Nodes WHERE Node_Id = " + toString(node_id));
  return !result.empty();
}

void updateHistorized(work* transaction, const string& target_node_id) {
  auto timestamp = getCurrentTimestamp();
  transaction->exec("UPDATE Historized_Nodes Last_Updated = $2 WHERE "
                    "Node_ID = $1",
      params{target_node_id, timestamp});
  transaction->commit();
}

UA_StatusCode Historizer::registerNodeId(
    UA_Server* server, UA_NodeId node_id, const UA_DataType* type) {
  auto target = toString(&node_id);
  try {
    auto session = connect();
    work transaction(session);
    if (isHistorized(&node_id)) {
      updateHistorized(&transaction, target);
    } else {
      auto timestamp = getCurrentTimestamp();
      transaction.exec("INSERT INTO Historized_Nodes(Node_Id, Last_Updated)  "
                       "VALUES($1, $2)",
          params{target, timestamp});
    }
    transaction.commit();

    auto table_name = toSanitizedString(&node_id);
    auto value_type = toPostgreDataType(type);
    transaction.exec("CREATE TABLE IF NOT EXISTS $1("
                     "Index BIGSERIAL PRIMARY KEY, "
                     "Server_Timestamp TIMESTAMP NOT NULL, "
                     "Source_Timestamp TIMESTAMP NOT NULL, "
                     "Value $2 NOT NULL);",
        params{
            table_name, value_type}); // if table exists, check value data type
    transaction.commit();

    auto monitor_request = UA_MonitoredItemCreateRequest_default(node_id);
    // NOLINTNEXTLINE(readability-magic-numbers)
    monitor_request.requestedParameters.samplingInterval = 1000000.0;
    monitor_request.monitoringMode = UA_MONITORINGMODE_REPORTING;
    auto result = UA_Server_createDataChangeMonitoredItem(server,
        UA_TIMESTAMPSTORETURN_BOTH, monitor_request, nullptr,
        &Historizer::dataChanged);
    return result.statusCode;
  } catch (const ConnectionUnavailable& ex) {
    log(SeverityLevel::Critical,
        "Could not read {} value. Database connection is not available",
        target);
    return UA_STATUSCODE_BADDATAUNAVAILABLE;
  } catch (const exception& ex) {
    log(SeverityLevel::Critical,
        "An unhandled exception occurred while trying to register {} node. "
        "Exception: {}",
        target, ex.what());
    return UA_STATUSCODE_BADUNEXPECTEDERROR;
  }
}

UA_HistoryDatabase Historizer::createDatabase() {
  UA_HistoryDatabase database;
  memset(&database, 0, sizeof(UA_HistoryDatabase));

  // open62541 uses database context to check if historization service is
  // available, so we MUST set some value
  database.context = (UA_Boolean*)UA_calloc(1, sizeof(UA_Boolean));
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

void Historizer::clear(UA_HistoryDatabase* database) {
  if (database == nullptr || database->context == nullptr) {
    return;
  } else {
    // cleanup dummy context
    auto* ctx = (UA_Boolean*)database->context;
    UA_free(ctx);
  }
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

void addNodeValue(params* values, UA_Variant variant) {
  switch (variant.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    auto value = *((bool*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    [[fallthrough]]; // pqxx does not allow signed char as a parameter
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    [[fallthrough]]; // pqxx does not allow unsigned char as a parameter
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = *((UA_UInt16*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = *((UA_Int16*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = *((UA_UInt32*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = *((UA_Int32*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = *((UA_UInt64*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = *((UA_Int64*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    auto value = *((UA_StatusCode*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = *((UA_Float*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = *((UA_Double*)(variant.data));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    auto value = getTimestamp(*((UA_DateTime*)(variant.data)));
    values->append(value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    auto* byte_string = (UA_ByteString*)(variant.data);
    values->append(binary_cast(byte_string->data, byte_string->length));
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    auto* ua_string = (UA_String*)(variant.data);
    auto value = string((char*)ua_string->data, ua_string->length);
    values->append(value);
    break;
  }
  default: {
    string error_msg =
        "Unhandled UA_Variant type detected: " + string(variant.type->typeName);
    throw logic_error(error_msg);
  }
  }
}

void Historizer::setValue(UA_Server* /*server*/, void* /*hdb_context*/,
    const UA_NodeId* /*session_id*/, void* /*session_context*/,
    const UA_NodeId* node_id, UA_Boolean historizing,
    const UA_DataValue* value) {
  auto target = toString(node_id);
  if (!historizing) {
    log(SeverityLevel::Info, "Node {} is not configured for historization ",
        target);
    return;
  }

  if (value == nullptr) {
    log(SeverityLevel::Error,
        "Failed to historize Node {} value. No data provided.", target);
    return;
  }

  try {
    auto session = connect();
    work transaction(session);

    string source_time;
    if (value->hasSourceTimestamp) {
      source_time = getTimestamp(value->sourceTimestamp);
    }
    string server_time;
    if (value->hasServerTimestamp) {
      server_time = getTimestamp(value->serverTimestamp);
    }
    auto node_id_string = toSanitizedString(node_id);
    params values{node_id_string, source_time, server_time};
    addNodeValue(&values, value->value);

    transaction.exec(
        "INSERT INTO $1(Source_Timestamp, Server_Timestamp, Value) "
        "VALUES($2, $3, 4)",
        values);
    transaction.commit();

    updateHistorized(&transaction, target);
  } catch (const ConnectionUnavailable& ex) {
    log(SeverityLevel::Critical,
        "Could not read {} value. Database connection is not available",
        target);
  } catch (exception& ex) {
    log(SeverityLevel::Error,
        "Failed to historize Node {} value due to an exception. "
        "Exception: {}",
        toString(node_id), ex.what());
  }
}

void Historizer::dataChanged(UA_Server* server, UA_UInt32 /*monitored_item_id*/,
    void* /*monitored_item_context*/, const UA_NodeId* node_id,
    void* /*node_context*/, UA_UInt32 attribute_id, const UA_DataValue* value) {
  // obtain session id, its set to nullptr in the
  // example code, so might be imposable to do so
  UA_NodeId* session_id = nullptr;
  UA_Boolean historize = false;
  if ((attribute_id & UA_ATTRIBUTEID_HISTORIZING) != 0) {
    historize = true;
  }

  setValue(server, nullptr, session_id, nullptr, node_id, historize, value);
}

string setColumnNames(UA_TimestampsToReturn timestamps_to_return) {
  string result = "Index, Value";
  if (timestamps_to_return != UA_TIMESTAMPSTORETURN_NEITHER) {
    if (timestamps_to_return == UA_TIMESTAMPSTORETURN_SOURCE ||
        timestamps_to_return == UA_TIMESTAMPSTORETURN_BOTH) {
      result += ", Source_Timestamp";
    }
    if (timestamps_to_return == UA_TIMESTAMPSTORETURN_SERVER ||
        timestamps_to_return == UA_TIMESTAMPSTORETURN_BOTH) {
      result += ", Server_Timestamp";
    }
  }
  return result;
}

string setColumnFilters(
    UA_Boolean include_bounds, UA_DateTime start_time, UA_DateTime end_time) {
  UA_DateTime start, end;
  if (start_time < end_time) {
    start = start_time;
    end = end_time;
  } else { // set reverse order filters
    start = end_time;
    end = start_time;
  }

  string lower_filter = ">";
  string upper_filter = "<";
  if (include_bounds) {
    lower_filter += "=";
    upper_filter += "=";
  }

  string result = "WHERE";
  if (start > 0) {
    result += " Source_Timestamp " + lower_filter + getTimestamp(start);
    if (end > 0) {
      result += " AND ";
    }
  }
  if (end > 0) {
    result += " Source_Timestamp " + upper_filter + getTimestamp(end);
  }

  return result != "WHERE" ? result : "";
}

string setColumnFilters(UA_Boolean include_bounds, UA_DateTime start_time,
    UA_DateTime end_time, const UA_ByteString* continuation_point) {
  auto result = setColumnFilters(include_bounds, start_time, end_time);

  if (continuation_point != nullptr) {
    auto continuation_index =
        string((char*)continuation_point->data, continuation_point->length);
    if (!continuation_index.empty()) {
      if (!result.empty()) {
        result += " AND";
      } else {
        result = "WHERE";
      }
      result += "Index > " + continuation_index;
    }
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

UA_DateTime toUaDateTime(const string& data) {
  using namespace date;
  using namespace chrono;

  istringstream string_stream{data};
  system_clock::time_point time_point;
  string_stream >> parse("%F %T", time_point);

  UA_DateTimeStruct calendar_time;
  auto day_point = floor<days>(time_point);
  auto calender_date = year_month_day{day_point};
  // NOLINTNEXTLINE(bugprone-narrowing-conversions)
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
}

UA_Variant toUaVariant(const field& data,
    const unordered_map<int64_t, UA_DataTypeKind>& type_map) {
  UA_Variant result;
  UA_Variant_init(&result);
  auto type = type_map.at(data.type());
  switch (type) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    auto value = data.as<bool>();
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = static_cast<uint8_t>(data.as<int>());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = static_cast<int8_t>(data.as<int>());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_SBYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = static_cast<uint16_t>(data.as<int>());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = static_cast<int16_t>(data.as<int>());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = static_cast<uint32_t>(data.as<long>());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = data.as<int32_t>();
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = static_cast<uint64_t>(data.as<long>());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = data.as<long>();
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    auto value = data.as<int>();
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_STATUSCODE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = data.as<float>();
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = data.as<double>();
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    auto timestamp = data.as<string>();
    auto value = toUaDateTime(timestamp);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DATETIME]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    auto opaque = binarystring(data);
    UA_ByteString value;
    value.length = opaque.size();
    value.data = (UA_Byte*)malloc(value.length);
    memcpy(value.data, opaque.bytes(), value.length);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTESTRING]);
    UA_String_clear(&value);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    auto cpp_string = data.as<string>();
    UA_String value = UA_String_fromChars(cpp_string.c_str());
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&value);
    break;
  }
  default: {
    throw logic_error("Unhandled UA Data Type " + std::to_string(type));
  }
  }

  return result;
}

UA_StatusCode expandHistoryResult(
    UA_HistoryData* result, vector<Historizer::ResultType> rows) {
  auto* data =
      (UA_DataValue*)UA_Array_new(rows.size(), &UA_TYPES[UA_TYPES_DATAVALUE]);
  if (data == nullptr) {
    return UA_STATUSCODE_BADOUTOFMEMORY;
  }

  for (size_t index = 0; index < rows.size(); ++index) {
    data[index].hasValue = true;
    data[index].hasSourceTimestamp = false;
    data[index].hasServerTimestamp = false;
    data[index].value = rows[index].value;
    if (!rows[index].source_timestamp.empty()) {
      data[index].hasSourceTimestamp = true;
      data[index].sourceTimestamp = toUaDateTime(rows[index].source_timestamp);
    }
    if (!rows[index].server_timestamp.empty()) {
      data[index].hasServerTimestamp = true;
      data[index].sourceTimestamp = toUaDateTime(rows[index].server_timestamp);
    }
  }

  return appendUADataValue(result, data, rows.size());
}

/**
 * @todo: expand continuation_point to store either the index of next value
 or
 * a future result for async select statements
 */
UA_ByteString* makeContinuationPoint(size_t last_index) {
  string index = to_string(last_index);
  UA_ByteString* result = UA_ByteString_new(); // should it be on heap?
  auto status = UA_ByteString_allocBuffer(result, index.size());
  if (status == UA_STATUSCODE_GOOD) {
    memcpy(result->data, index.c_str(), index.size());
  } else {
    throw OutOfMemory();
  }
  return result;
}

Historizer::ResultType makeResultType(const pqxx::row& entry,
    UA_TimestampsToReturn timestamps_to_return,
    const unordered_map<int64_t, UA_DataTypeKind>& type_map) {
  Historizer::ResultType result{// clang-format off
      .index = entry["Index"].as<long>(),
      .value = toUaVariant(entry["Value"], type_map),
      .source_timestamp = "", 
      .server_timestamp = ""
    }; // clang-format on
  if (timestamps_to_return != UA_TIMESTAMPSTORETURN_NEITHER) {
    if (timestamps_to_return == UA_TIMESTAMPSTORETURN_SOURCE ||
        timestamps_to_return == UA_TIMESTAMPSTORETURN_BOTH) {
      result.source_timestamp = entry["Source_Timestamp"].as<string>();
    }
    if (timestamps_to_return == UA_TIMESTAMPSTORETURN_SERVER ||
        timestamps_to_return == UA_TIMESTAMPSTORETURN_BOTH) {
      result.server_timestamp = entry["Server_Timestamp"].as<string>();
    }
  }
  return result;
}

vector<Historizer::ResultType> makeResultTypes(const pqxx::result& rows,
    UA_TimestampsToReturn timestamps_to_return,
    const unordered_map<int64_t, UA_DataTypeKind>& type_map) {
  vector<Historizer::ResultType> results;
  for (const auto& row : rows) {
    results.push_back(makeResultType(row, timestamps_to_return, type_map));
  }
  return results;
}

vector<Historizer::ResultType> Historizer::readHistory(
    const UA_ReadRawModifiedDetails* history_read_details,
    UA_UInt32 /*timeout_hint*/, UA_TimestampsToReturn timestamps_to_return,
    UA_NodeId node_id, const UA_ByteString* continuation_point_in,
    [[maybe_unused]] UA_ByteString* continuation_point_out) { // NOLINT

  auto columns = setColumnNames(timestamps_to_return);
  auto filters = setColumnFilters(history_read_details->returnBounds,
      history_read_details->startTime, history_read_details->endTime,
      continuation_point_in);

  auto read_limit = history_read_details->numValuesPerNode;
  string result_order = "ORDER BY Source_Timestamp ";
  if (((history_read_details->startTime == 0) &&
          ((read_limit != 0) && history_read_details->endTime != 0)) ||
      (history_read_details->startTime > history_read_details->endTime)) {
    // if start time was not specified (start time is equal to
    // DateTime.MinValue), but end time AND read_limit is, OR end time is
    // less than start time, than the historical values should be in reverse
    // order (freshest values first, oldest ones last)
    result_order += "DESC";
  } else {
    result_order += "ASC";
  }

  auto session = connect();
  work transaction(session);
  auto table = toSanitizedString(&node_id);
  string query =
      "SELECT " + columns + " FROM " + table + filters + result_order;

  if (read_limit != 0) { // if numValuesPerNode is zero, there is no limit
    query += " FETCH FIRST " + to_string(read_limit) + " ROWS ONLY";
  }

  auto rows = transaction.exec(query);
  auto results = makeResultTypes(rows, timestamps_to_return, type_map_);

  if (read_limit != 0 && results.size() == read_limit) {
    // check if there overrun
    auto record_count =
        transaction.exec("SELECT COUNT(*) FROM " + table + filters)
            .expect_rows(1)
            .at(0)
            .at(0)
            .as<long>();
    if (record_count > read_limit) {
      auto index = results.back().index;
      // continuation_point_out is read by the Client, not the Server
      continuation_point_out = makeContinuationPoint(index);
    }
  }
  return results;
}
// NOLINTNEXTLINE parameter names are set by open62541
void Historizer::readRaw(UA_Server* /*server*/, void* /*hdb_context*/,
    const UA_NodeId* /*session_id*/, void* /*session_context*/,
    const UA_RequestHeader* request_header,
    const UA_ReadRawModifiedDetails* history_read_details,
    UA_TimestampsToReturn timestamps_to_return,
    UA_Boolean release_continuation_points, size_t nodes_to_read_size,
    const UA_HistoryReadValueId* nodes_to_read,
    UA_HistoryReadResponse* response,
    UA_HistoryData* const* const history_data) {
  response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
  if (!release_continuation_points) {
    for (size_t i = 0; i < nodes_to_read_size; ++i) {
      try {
        auto history_values = readHistory(history_read_details,
            request_header->timeoutHint, timestamps_to_return,
            nodes_to_read[i].nodeId, &nodes_to_read[i].continuationPoint,
            &response->results[i].continuationPoint);
        response->results[i].statusCode =
            expandHistoryResult(history_data[i], history_values);
      } catch (const BadContinuationPoint& ex) {
        response->results[i].statusCode =
            UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
      } catch (const ConnectionUnavailable& ex) {
        response->responseHeader.serviceResult =
            UA_STATUSCODE_BADDATAUNAVAILABLE;
      } catch (const OutOfMemory& ex) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
      } catch (const runtime_error& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
  }
  /* Continuation points are not stored internally, so no need to release
   * them, simply return StatusCode::Good for the entire request without
   * setting any data*/
}

template <typename T> T multiply(const UA_Variant& lhs, const intmax_t& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  if (lhs_value > 0 && rhs > 0) {
    if (lhs_value > numeric_limits<T>::max() / rhs) {
      // overflow
      return numeric_limits<T>::max();
    }
  }

  if (lhs_value < 0 && rhs < 0) {
    if (lhs_value < numeric_limits<T>::min() / rhs) {
      // overflow
      return numeric_limits<T>::max();
    }
  }

  if (lhs_value < 0 && rhs > 0) {
    if (rhs < numeric_limits<T>::min() / lhs_value) {
      // underflow
      return numeric_limits<T>::min();
    }
  }

  if (lhs_value > 0 && rhs < 0) {
    if (lhs_value < numeric_limits<T>::min() / rhs) {
      // underflow
      return numeric_limits<T>::min();
    }
  }

  T value = round(lhs_value * rhs);
  return value;
}

UA_Variant operator*(const UA_Variant& lhs, const intmax_t& rhs) {
  if (lhs.type->typeKind == UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN) {
    throw logic_error("Can not multiply Boolean values");
  }

  UA_Variant result;
  UA_Variant_init(&result);
  switch (lhs.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = multiply<UA_SByte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_SBYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = multiply<UA_Byte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = multiply<UA_UInt16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = multiply<UA_Int16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = multiply<UA_UInt32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = multiply<UA_Int32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = multiply<UA_UInt64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = multiply<UA_Int64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = multiply<float>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = multiply<double>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    break;
  }
  default: {
    throw logic_error("Can not multiply non numeric data");
  }
  }
  return result;
}

template <typename T>
T divideUnsigned(const UA_Variant& lhs, const intmax_t& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  if (rhs == 0) {
    throw logic_error("Division by 0 is not allowed");
  }
  T value = round(lhs_value / rhs);
  return value;
}

template <typename T>
T divideSigned(const UA_Variant& lhs, const intmax_t& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  if (rhs == 0) {
    throw logic_error("Division by 0 is not allowed");
  }
  if (lhs_value == -1 && rhs == numeric_limits<intmax_t>::min()) {
    // overflow
    return numeric_limits<T>::max();
  }
  T value = round(lhs_value / rhs);
  return value;
}

UA_Variant operator/(const UA_Variant& lhs, const intmax_t& rhs) {
  if (lhs.type->typeKind == UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN) {
    throw logic_error("Can not divide Boolean values");
  }

  UA_Variant result;
  UA_Variant_init(&result);
  switch (lhs.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = divideSigned<UA_SByte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_SBYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = divideUnsigned<UA_Byte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = divideUnsigned<UA_UInt16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = divideSigned<UA_Int16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = divideUnsigned<UA_UInt32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = divideSigned<UA_Int32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = divideUnsigned<UA_UInt64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = divideSigned<UA_Int64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = divideSigned<float>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = divideSigned<double>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    break;
  }
  default: {
    throw logic_error("Can not divide non numeric data");
  }
  }
  return result;
}

template <typename T>
T sumUnsigned(const UA_Variant& lhs, const UA_Variant& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  auto rhs_value = *((T*)(rhs.data));
  auto value = lhs_value + rhs_value;
  if (value < lhs_value) {
    // overflow
    return numeric_limits<T>::max();
  }
  return value;
}

template <typename T>
T sumSigned(const UA_Variant& lhs, const UA_Variant& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  auto rhs_value = *((T*)(rhs.data));
  if (lhs_value > numeric_limits<T>::max() - rhs_value) {
    // overflow
    return numeric_limits<T>::max();
  }
  if (lhs_value < numeric_limits<T>::min() + rhs_value) {
    // underflow
    return numeric_limits<T>::min();
  }
  T value = lhs_value + rhs_value;
  return value;
}

UA_Variant operator+(const UA_Variant& lhs, const UA_Variant& rhs) {
  if (lhs.type->typeKind != rhs.type->typeKind) {
    throw logic_error("Can not not add non matching numeric types");
  }

  if (lhs.type->typeKind == UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN) {
    throw logic_error("Can not add Boolean values");
  }

  UA_Variant result;
  UA_Variant_init(&result);

  switch (lhs.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = sumUnsigned<UA_SByte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_SBYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = sumSigned<UA_Byte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = sumSigned<UA_UInt16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = sumUnsigned<UA_Int16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = sumSigned<UA_UInt32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = sumUnsigned<UA_Int32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = sumSigned<UA_UInt64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = sumUnsigned<UA_Int64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = sumUnsigned<float>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = sumUnsigned<double>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    break;
  }
  default: {
    throw logic_error("Can not add non numeric data");
  }
  }
  return result;
}

template <typename T>
T numericUnsignedDiff(const UA_Variant& lhs, const UA_Variant& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  auto rhs_value = *((T*)(rhs.data));
  // @todo: use modulo for diff instead?
  auto value = lhs_value - rhs_value;
  if (lhs_value < rhs_value) {
    // underflow
    return numeric_limits<T>::min();
  }
  return value;
}

template <typename T>
T numericSignedDiff(const UA_Variant& lhs, const UA_Variant& rhs) {
  auto lhs_value = *((T*)(lhs.data));
  auto rhs_value = *((T*)(rhs.data));
  if (lhs_value > numeric_limits<T>::max() - rhs_value) {
    // overflow
    return numeric_limits<T>::max();
  }
  if (lhs_value < numeric_limits<T>::min() + rhs_value) {
    // underflow
    return numeric_limits<T>::min();
  }

  T value = lhs_value - rhs_value;
  return value;
}

UA_Variant operator-(const UA_Variant& lhs, const UA_Variant& rhs) {
  if (lhs.type->typeKind != rhs.type->typeKind) {
    throw logic_error("Can not not subtract non matching numeric types");
  }

  if (lhs.type->typeKind == UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN) {
    throw logic_error("Can not subtract Boolean values");
  }

  UA_Variant result;
  UA_Variant_init(&result);

  switch (lhs.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    auto value = numericSignedDiff<UA_SByte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_SBYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    auto value = numericUnsignedDiff<UA_Byte>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BYTE]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    auto value = numericUnsignedDiff<UA_UInt16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    auto value = numericSignedDiff<UA_Int16>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT16]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    auto value = numericUnsignedDiff<UA_UInt32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    auto value = numericSignedDiff<UA_Int32>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT32]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    auto value = numericUnsignedDiff<UA_UInt64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    auto value = numericSignedDiff<UA_Int64>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT64]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    auto value = numericSignedDiff<float>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_FLOAT]);
    break;
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    auto value = numericSignedDiff<double>(lhs, rhs);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
    break;
  }
  default: {
    throw logic_error("Can not subtract non numeric data");
  }
  }
  return result;
}

Historizer::ResultType interpolateValues(UA_DateTime target_timestamp,
    const Historizer::ResultType& before, const Historizer::ResultType& after) {
  auto before_timestamp = toUaDateTime(before.server_timestamp);
  auto after_timestamp = toUaDateTime(after.server_timestamp);
  // use formula from OPC UA Part 13 Aggregates specification section 3.1.8
  intmax_t weight = target_timestamp - before_timestamp;
  auto value_diff = after.value - before.value;
  intmax_t denominator = after_timestamp - before_timestamp;
  auto interpolated = ((value_diff * weight) / denominator) + before.value;

  return Historizer::ResultType{.index = -1,
      .value = interpolated,
      .source_timestamp = getTimestamp(target_timestamp),
      .server_timestamp = getTimestamp(target_timestamp)};
}

/**
 * @brief Defines Aggregate Result Status Code expansion
 *
 * Defined in UA Part 4: Services Table 181 DataValue InfoBits
 *
 */
namespace HistorianBits {
enum class DataLocation : uint8_t {
  Raw = 0x00,
  Calculated = 0x01,
  Interpolated = 0x02,
  Reserved = 0x03
};

void setHistorianBits(UA_StatusCode* status, DataLocation data_loc,
    bool is_partial = false, bool has_extra = false,
    bool has_multiple = false) {
  if (UA_StatusCode_isBad(*status)) {
    string error_msg =
        "Can not set historian bits for " + string(UA_StatusCode_name(*status));
    throw invalid_argument(error_msg);
  }
  if (data_loc == DataLocation::Reserved) {
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
  case DataLocation::Raw: {
    return "Raw";
  }
  case DataLocation::Calculated: {
    return "Calculated";
  }
  case DataLocation::Interpolated: {
    return "Interpolated";
  }
  default: {
    return "Reserved";
  }
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
    const UA_ReadAtTimeDetails* history_read_details,
    UA_UInt32 /*timeout_hint*/, UA_TimestampsToReturn timestamps_to_return,
    UA_NodeId node_id, const UA_ByteString* /*continuation_point_in*/,
    [[maybe_unused]] UA_ByteString* continuation_point_out,
    UA_HistoryData* history_data) {

  auto columns = setColumnNames(timestamps_to_return);
  UA_StatusCode status = UA_STATUSCODE_GOOD;
  using namespace HistorianBits;
  vector<ResultType> results;
  for (size_t i = 0; i < history_read_details->reqTimesSize; ++i) {
    auto timestamp = getTimestamp(history_read_details->reqTimes[i]);
    auto session = connect();
    work transaction(session);
    auto table = toSanitizedString(&node_id);
    string query = "SELECT " + columns + " FROM " + table +
        " WHERE Source_Timestamp =" + timestamp +
        " ORDER BY Source_Timestamp ASC";
    /**
     * @todo: use an async select request or a batch read, and check if
     * timeout_hint elapsed, if it did set a continuation point
     */
    auto rows = transaction.exec(query);
    if (rows.empty()) {
      string nearest_query = "SELECT " + columns + " FROM " + table +
          " WHERE Source_Timestamp $1 " + timestamp +
          " ORDER BY Source_Timestamp ASC LIMIT 1;";
      auto nearest_before = makeResultType(
          transaction.exec(nearest_query, params{"<"}).expect_rows(1).at(0),
          timestamps_to_return, type_map_);
      auto nearest_after = makeResultType(
          transaction.exec(nearest_query, params{">"}).expect_rows(1).at(0),
          timestamps_to_return, type_map_);
      // we do not check history_read_details->useSimpleBoundsflag,
      // because all values are Non-Bad for our case and thus
      // useSimpleBounds=False would not change the calculation
      auto interpolated = interpolateValues(
          history_read_details->reqTimes[i], nearest_before, nearest_after);
      results.push_back(interpolated);
      setHistorianBits(&status, DataLocation::Interpolated);
    } else {
      auto raw = makeResultTypes(rows, timestamps_to_return, type_map_);
      results.insert(results.end(), raw.begin(), raw.end());
      setHistorianBits(&status, DataLocation::Raw);
    }
  }
  expandHistoryResult(history_data, results);
  return status;
}

void Historizer::readAtTime(UA_Server* /*server*/, void* /*hdb_context*/,
    const UA_NodeId* /*session_id*/, void* /*session_context*/,
    const UA_RequestHeader* request_header,
    const UA_ReadAtTimeDetails* history_read_details,
    UA_TimestampsToReturn timestamps_to_return,
    UA_Boolean release_continuation_points, size_t nodes_to_read_size,
    const UA_HistoryReadValueId* nodes_to_read,
    UA_HistoryReadResponse* response,
    UA_HistoryData* const* const
        history_data) { // NOLINT parameter name set by open62541
  response->responseHeader.serviceResult = UA_STATUSCODE_GOOD;
  if (!release_continuation_points) {
    for (size_t i = 0; i < nodes_to_read_size; ++i) {
      try {
        response->results[i].statusCode =
            readAndAppendHistory(history_read_details,
                request_header->timeoutHint, timestamps_to_return,
                nodes_to_read[i].nodeId, &nodes_to_read[i].continuationPoint,
                &response->results[i].continuationPoint, history_data[i]);
      } catch (logic_error& ex) {
        // Target Node Id values can not be interpolated due to mismatching
        // data types or non aggregable data types (text, opaque values,
        // etc...) OR resulting timestamps where malformed and could not be
        // decoded as UA_DateTime
        response->results[i].statusCode =
            UA_STATUSCODE_BADAGGREGATEINVALIDINPUTS;
      } catch (const NoBoundData& ex) {
        response->results[i].statusCode = UA_STATUSCODE_BADBOUNDNOTFOUND;
      } catch (const NoData& ex) {
        response->results[i].statusCode = UA_STATUSCODE_BADNODATA;
      } catch (const BadContinuationPoint& ex) {
        response->results[i].statusCode =
            UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
      } catch (const ConnectionUnavailable& ex) {
        response->responseHeader.serviceResult =
            UA_STATUSCODE_BADDATAUNAVAILABLE;
      } catch (const OutOfMemory& ex) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
      } catch (const runtime_error& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
  }
}
} // namespace open62541
#endif // UA_ENABLE_HISTORIZING