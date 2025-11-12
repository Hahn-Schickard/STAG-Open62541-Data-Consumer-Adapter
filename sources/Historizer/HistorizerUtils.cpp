#include "HistorizerUtils.hpp"
#include "Exceptions.hpp"
#include "StringConverter.hpp"
#include "UaVariantOperators.hpp"

#include <date/date.h>

#include <chrono>
#include <cmath>

namespace open62541 {
using namespace std;
using namespace pqxx;

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

TypeMap queryTypeOIDs(work* transaction) {
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
  TypeMap result;
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

string getCurrentTimestamp() {
  auto timestamp = chrono::system_clock::now();
  return date::format("%F %T", timestamp);
}

string toSanitizedString(const UA_NodeId* node_id) {
  return "\"" + toString(node_id) + "\"";
}

void addNodeValue(params* values, UA_Variant variant) {
  switch (variant.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    auto value = *((bool*)(variant.data));
    values->append(value);
    break;
  }
  // NOLINTNEXTLINE(bugprone-branch-clone)
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
    auto value = toString(*((UA_DateTime*)(variant.data)));
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
    result += " Source_Timestamp " + lower_filter + toString(start);
    if (end > 0) {
      result += " AND ";
    }
  }
  if (end > 0) {
    result += " Source_Timestamp " + upper_filter + toString(end);
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
  calendar_time.year = static_cast<UA_Int16>(int{calender_date.year()});
  calendar_time.month = static_cast<UA_UInt16>(unsigned{calender_date.month()});
  calendar_time.day = static_cast<UA_UInt16>(unsigned{calender_date.day()});

  auto day_time = hh_mm_ss{time_point - day_point};
  calendar_time.hour = static_cast<UA_UInt16>(day_time.hours().count());
  calendar_time.min = static_cast<UA_UInt16>(day_time.minutes().count());
  calendar_time.sec = static_cast<UA_UInt16>(day_time.seconds().count());
  calendar_time.milliSec =
      static_cast<UA_UInt16>(floor<milliseconds>(day_time.seconds()).count());
  calendar_time.microSec =
      static_cast<UA_UInt16>(floor<microseconds>(day_time.seconds()).count());
  calendar_time.nanoSec =
      static_cast<UA_UInt16>(floor<nanoseconds>(day_time.seconds()).count());

  return UA_DateTime_fromStruct(calendar_time);
}

UA_Variant toUaVariant(const field& data, const TypeMap& type_map) {
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

UA_ByteString* makeContinuationPoint(intmax_t last_index) {
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

HistoryResults makeHistoryResults(const result& rows,
    UA_TimestampsToReturn timestamps_to_return, const TypeMap& type_map) {
  HistoryResults result;
  for (const auto& row : rows) {
    result.push_back(makeHistoryResult(row, timestamps_to_return, type_map));
  }
  return result;
}

HistoryResult makeHistoryResult(const row& entry,
    UA_TimestampsToReturn timestamps_to_return, const TypeMap& type_map) {
  HistoryResult result{// clang-format off
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

UA_StatusCode expandHistoryResult(
    UA_HistoryData* result, const HistoryResults& rows) {
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

HistoryResult interpolateValues(UA_DateTime target_timestamp,
    const HistoryResult& before, const HistoryResult& after) {
  auto before_timestamp = toUaDateTime(before.server_timestamp);
  auto after_timestamp = toUaDateTime(after.server_timestamp);
  // use formula from OPC UA Part 13 Aggregates specification section 3.1.8
  intmax_t weight = target_timestamp - before_timestamp;
  auto value_diff = after.value - before.value;
  intmax_t denominator = after_timestamp - before_timestamp;
  auto interpolated = ((value_diff * weight) / denominator) + before.value;

  return HistoryResult{.index = -1,
      .value = interpolated,
      .source_timestamp = toString(target_timestamp),
      .server_timestamp = toString(target_timestamp)};
}

} // namespace open62541