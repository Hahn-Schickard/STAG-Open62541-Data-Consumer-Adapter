#include "UaVariantOperators.hpp"

#include <cmath>
#include <stdexcept>

namespace open62541 {
using namespace std;

#ifdef __GNUC__
#pragma GCC diagnostic push // suppres conversion warnings for this section
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

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
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

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
} // namespace open62541