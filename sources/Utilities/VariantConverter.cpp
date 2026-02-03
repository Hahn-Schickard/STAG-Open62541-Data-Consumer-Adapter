#include "VariantConverter.hpp"
#include "StringConverter.hpp"

#include <cmath>
#include <stdexcept>

namespace open62541 {
using namespace std;
using namespace Information_Model;

UA_NodeId toNodeId(DataType type) {
  switch (type) {
  case DataType::Boolean: {
    return UA_TYPES[UA_TYPES_BOOLEAN].typeId;
  }
  case DataType::Unsigned_Integer: {
    return UA_TYPES[UA_TYPES_UINT64].typeId;
  }
  case DataType::Integer: {
    return UA_TYPES[UA_TYPES_INT64].typeId;
  }
  case DataType::Double: {
    return UA_TYPES[UA_TYPES_DOUBLE].typeId;
  }
  case DataType::Opaque: {
    return UA_TYPES[UA_TYPES_BYTESTRING].typeId;
  }
  case DataType::String: {
    return UA_TYPES[UA_TYPES_STRING].typeId;
  }
  case DataType::Timestamp: {
    return UA_TYPES[UA_TYPES_DATETIME].typeId;
  }
  case DataType::Unknown:
  default: {
    throw runtime_error("Could not convert STAG Information Model Data Type to "
                        "OPC UA data type!");
  }
  }
}

// In this case, code is easier to understand WITH magic numbers
// NOLINTBEGIN(readability-magic-numbers)
UA_Variant toUAVariant(const DataVariant& variant) {
  // @todo: refactor into to avoid callers having to create another copy:
  // void toUAVariant(const DataVariant& variant, UA_Variant*)
  UA_Variant result;
  UA_Variant_init(&result);
  if (holds_alternative<bool>(variant)) {
    auto value = get<bool>(variant);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
  } else if (holds_alternative<uintmax_t>(variant)) {
    auto value = get<uintmax_t>(variant);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_UINT64]);
  } else if (holds_alternative<intmax_t>(variant)) {
    auto value = get<intmax_t>(variant);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_INT64]);
  } else if (holds_alternative<double>(variant)) {
    auto value = get<double>(variant);
    UA_Variant_setScalarCopy(&result, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
  } else if (holds_alternative<Timestamp>(variant)) {
    auto value = get<Timestamp>(variant);
    UA_DateTimeStruct date_time_struct;
    date_time_struct.year = static_cast<UA_Int16>(value.year);
    date_time_struct.month = (UA_UInt16)value.month;
    date_time_struct.day = (UA_UInt16)value.day;
    date_time_struct.hour = (UA_UInt16)value.hours;
    date_time_struct.min = (UA_UInt16)value.minutes;
    date_time_struct.sec = (UA_UInt16)value.seconds;
    date_time_struct.milliSec = (UA_UInt16)(floor(value.microseconds / 1000));
    date_time_struct.microSec = (UA_UInt16)(value.microseconds % 1000);
    date_time_struct.nanoSec = 0;
    auto date_time = UA_DateTime_fromStruct(date_time_struct);
    UA_Variant_setScalarCopy(&result, &date_time, &UA_TYPES[UA_TYPES_DATETIME]);
  } else if (holds_alternative<string>(variant)) {
    auto ua_string = makeUAString(get<string>(variant));
    UA_Variant_setScalarCopy(&result, &ua_string, &UA_TYPES[UA_TYPES_STRING]);
    UA_String_clear(&ua_string);
  } else if (holds_alternative<vector<uint8_t>>(variant)) {
    auto ua_byte_string = makeUAByteString(get<vector<uint8_t>>(variant));
    UA_Variant_setScalarCopy(
        &result, &ua_byte_string, &UA_TYPES[UA_TYPES_BYTESTRING]);
    UA_String_clear(&ua_byte_string);
  } else {
    throw runtime_error("Could not convert Information_Model::DataVariant into "
                        "open62541 UA_Variant due to an unhandled "
                        "Information_Model::DataVariant value type");
  }

  return result;
}

DataVariant toDataVariant(const UA_Variant& variant) {
  switch (variant.type->typeKind) {
  case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
    bool value = *((bool*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE: {
    // NOLINTNEXTLINE(bugprone-signed-char-misuse,cert-str34-c)
    intmax_t value = *((UA_SByte*)(variant.data)); // this is not a char
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
    intmax_t value = *((UA_Int16*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
    intmax_t value = *((UA_Int32*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
    intmax_t value = *((UA_Int64*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME: {
    UA_DateTimeStruct time_value =
        UA_DateTime_toStruct(*(UA_DateTime*)(variant.data));
    Timestamp value{.year = static_cast<uint16_t>(time_value.year),
        .month = static_cast<uint8_t>(time_value.month),
        .day = static_cast<uint8_t>(time_value.day),
        .hours = static_cast<uint8_t>(time_value.hour),
        .minutes = static_cast<uint8_t>(time_value.min),
        .seconds = static_cast<uint8_t>(time_value.sec),
        .microseconds = static_cast<uint32_t>(
            (time_value.milliSec * 1000) + time_value.microSec)};
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
    uintmax_t value = *((UA_Byte*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT16: {
    uintmax_t value = *((UA_UInt16*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT32: {
    uintmax_t value = *((UA_UInt32*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_UINT64: {
    uintmax_t value = *((UA_UInt64*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE: {
    UA_StatusCode status_code = *((UA_StatusCode*)(variant.data));
    auto value = string(UA_StatusCode_name(status_code));
    if (!value.empty()) {
      return DataVariant(value);
    } else {
      // if no human readable string is available, return the code as an integer
      return DataVariant(to_string(status_code));
    }
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
    double value = *((UA_Float*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
    double value = *((UA_Double*)(variant.data));
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING: {
    auto* byte_string = (UA_ByteString*)(variant.data);
    auto value = vector<uint8_t>(
        byte_string->data, byte_string->data + byte_string->length);
    return DataVariant(value);
  }
  case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
    auto* ua_string = (UA_String*)(variant.data);
    auto value = toString(ua_string);
    return DataVariant(value);
  }
  default: {
    string error_msg = string(variant.type->typeName) +
        " into Information_Model::DataVariant conversion is not supported";
    throw runtime_error(error_msg);
  }
  }
}
// NOLINTEND(readability-magic-numbers)
} // namespace open62541