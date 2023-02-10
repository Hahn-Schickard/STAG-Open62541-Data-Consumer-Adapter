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

using DataPoints = std::vector<DataType>;

// Restrict parameter pack data to only accept string like data types
template <typename... T>
// clang-format off
using OnlyStringTypes = typename enable_if<
        conjunction<is_convertible<T, string>...>::value>::type;
// clang-format on

// Unwinding method for variadic concatenate method
template <typename... T, typename = OnlyStringTypes<T...>>
std::string concatenate(
    const std::string& head, const std::string& tail, const T&... remainder) {
  return concatenate(head + tail, remainder...);
}

// Final concatenate method
std::string concatenate(const std::string& head, const std::string& tail) {
  return head + tail;
}

// Passthrough incase there is nothing to concatenate
std::string concatenate(const std::string& head) { return head; }

template <typename... T, typename = OnlyStringTypes<T...>>
nanodbc::result execute(const T&... query_parts) {
  return nanodbc::execute(*db_, concatenate(query_parts...));
}

enum class ColumnDataType {
  BOOLEAN, // 1 byte to store TRUE or FALSE
  INT, // 32 bit signed integer [-2^31;2^31]
  BIGINT, // 64 bit signed integer [-2^63;2^63]
  SMALLINT, // 16 bit signed integer [-2^15;2^15]
  DECIMAL, // N(p,s)
  FLOAT,
  CHAR, // c(n)
  VARCHAR, // c(n)
  DATE,
  TIME,
  TIMESTAMP,
  BIT, // Binary data of fixed length b(n)
  VARBIT, // variable length binary data b(n)
};

std::string toString(ColumnDataType type);

struct Column {
  Column(std::string name, ColumnDataType type);
  Column(std::string name, ColumnDataType type, bool null_allowed);
  Column(std::string name, ColumnDataType type, uint8_t size,
      bool null_allowed = false);
  Column(std::string name, ColumnDataType type, uint8_t precision,
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
}

using Columns = std::vector<Column>;

struct DatabaseDriver {
  DatabaseDriver();

  DatabaseDriver(const std::string& data_source);

  DatabaseDriver(const std::string& data_source, const std::string& username,
      const std::string& password);

  void create(const std::string& table_name, Columns columns);

  void drop(const std::string table_name);

  template <typename... T, typename = OnlyStringTypes<T...>>
  nanodbc::result execute(const T&... query_parts) {
    return nanodbc::execute(*db_, concatenate(query_parts...));
  }

  void insert(const std::string table_name,
      std::vector<std::string> column_names = {}, DataPoints data_points = {});

private:
  std::unique_ptr<nanodbc::connection> db_;
};
} // namespace ODD
#endif // __DATABASE_DRIVER_HPP
