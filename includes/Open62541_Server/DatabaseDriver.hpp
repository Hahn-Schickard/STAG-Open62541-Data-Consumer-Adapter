#ifndef __DATABASE_DRIVER_HPP
#define __DATABASE_DRIVER_HPP

#include <cstdint>
#include <memory>
#include <nanodbc/nanodbc.h>
#include <string>
#include <variant>
#include <vector>

namespace ODD {

// clang-format off
using DataType = std::variant<
                        bool, 
                        uintmax_t, 
                        intmax_t, 
                        float, 
                        double,
                        std::string, 
                        std::vector<uint8_t>
                    >;
// clang-format on

std::string toString(DataType data);

using DataPoints = std::vector<DataType>;

// Restrict parameter pack data to only accept string like data types
template <typename... T>
// clang-format off
using OnlyStringTypes = typename std::enable_if<
      std::conjunction<
        std::is_convertible<T, std::string>...>::value>::type;
// clang-format on

// Passthrough incase there is nothing to concatenate
inline std::string concatenate(const std::string& head) { return head; }

// Final concatenate function
inline std::string concatenate(
    const std::string& head, const std::string& tail) {
  return head + tail;
}

// Unwinding method for variadic concatenate function
template <typename... T, typename = OnlyStringTypes<T...>>
inline std::string concatenate(
    const std::string& head, const std::string& tail, const T&... remainder) {
  return concatenate(head + tail, remainder...);
}

enum class ColumnDataType : uint16_t {
  BOOLEAN = 0x100, // 1 byte to store TRUE or FALSE
  INT = 0x101, // 32 bit signed integer [-2^31;2^31]
  BIGINT = 0x102, // 64 bit signed integer [-2^63;2^63]
  SMALLINT = 0x103, // 16 bit signed integer [-2^15;2^15]
  DECIMAL = 0x200, // N(p,s)
  FLOAT = 0x201,
  CHAR = 0x300, // c(n)
  VARCHAR = 0x301, // c(n)
  BIT = 0x302, // Binary data of fixed length b(n)
  VARBIT = 0x303, // variable length binary data b(n)
  DATE = 0x400,
  TIME = 0x401,
  TIMESTAMP = 0x402
};

bool isIntegerType(ColumnDataType type);
bool isFloatingType(ColumnDataType type);
bool isVarLengthType(ColumnDataType type);
bool isFixedLengthType(ColumnDataType type);

std::string toString(ColumnDataType type);

struct Column {
  Column(const std::string& name, ColumnDataType type);
  Column(const std::string& name, ColumnDataType type, bool null_allowed);
  Column(const std::string& name, ColumnDataType type, uint8_t size,
      bool null_allowed = false);
  Column(const std::string& name, ColumnDataType type, uint8_t precision,
      uint8_t scale, bool null_allowed = false);

  std::string name();
  ColumnDataType type();
  bool isNullable();
  bool isCompressible();
  std::string toString();

private:
  std::string name_;
  ColumnDataType type_;
  bool null_allowed_ = false;
  uint8_t precision_ = 0; // used to specify decimal type precision
  uint8_t scale_ = 0; // used to specify the decimal type scale
  uint8_t size_ = 0; // Used to specify data type size for variable length data
                     // types like char and array
};

using Columns = std::vector<Column>;

struct ColumnValue {
  ColumnValue(const std::string& name, DataType value_);

  std::string name();
  std::string toString();
  DataType value();

private:
  std::string name_;
  DataType value_;
};

enum class FilterType {
  EQUAL,
  NOT_EQUAL,
  GREATER,
  LESS,
  GREATER_OR_EQUAL,
  LESS_OR_EQUAL,
  BETWEEN,
  NOT_BETWEEN,
  LIKE,
  NOT_LIKE,
  IN,
  NOT_IN
};

std::string toString(FilterType type);

struct ColumnFilter {
  ColumnFilter(FilterType type, const std::string column_name);
  ColumnFilter(FilterType type, const std::string column_name, DataType value);
  ColumnFilter(
      FilterType type, const std::string column_name, DataPoints values);

  FilterType type();
  std::string toString();
  DataPoints values();
  std::string column();

private:
  FilterType type_;
  std::string column_;
  DataPoints values_;
};

struct DatabaseDriver {
  DatabaseDriver();

  DatabaseDriver(const std::string& data_source);

  DatabaseDriver(const std::string& data_source, const std::string& username,
      const std::string& password);

  void create(const std::string& table_name, Columns columns);

  void drop(const std::string& table_name);

  template <typename... T, typename = OnlyStringTypes<T...>>
  nanodbc::result execute(const T&... query_parts) {
    return nanodbc::execute(*db_, concatenate(query_parts...));
  }

  void insert(const std::string& table_name, std::vector<ColumnValue> values);

private:
  std::unique_ptr<nanodbc::connection> db_;
};
} // namespace ODD
#endif // __DATABASE_DRIVER_HPP
