#include "DatabaseDriver.hpp"
#include "Variant_Visitor.hpp"

using namespace std;
using namespace nanodbc;

namespace ODD {

string toString(ColumnDataType type) {
  switch (type) {
  case ColumnDataType::BOOLEAN: {
    return "BOOLEAN";
  }
  case ColumnDataType::INT: {
    return "INTEGER";
  }
  case ColumnDataType::BIGINT: {
    return "BIGINT";
  }
  case ColumnDataType::SMALLINT: {
    return "SMALLINT";
  }
  case ColumnDataType::DECIMAL: {
    return "DECIMAL";
  }
  case ColumnDataType::FLOAT: {
    return "FLOAT";
  }
  case ColumnDataType::CHAR: {
    return "CHAR";
  }
  case ColumnDataType::VARCHAR: {
    return "VARCHAR";
  }
  case ColumnDataType::DATE: {
    return "DATE";
  }
  case ColumnDataType::TIME: {
    return "TIME";
  }
  case ColumnDataType::TIMESTAMP: {
    return "TIMESTAMP";
  }
  case ColumnDataType::BIT: {
    return "BIT";
  }
  case ColumnDataType::VARBIT: {
    return "VARBIT";
  }
  default: { throw logic_error("Given data type has no conversion string"); }
  }
}

static constexpr uint16_t COLUMN_DATA_TYPE_MASK = 0xFF00;
static constexpr uint8_t COLUMN_DATA_TYPE_OFFSET = 8;
static constexpr uint8_t COLUMN_DATA_TYPE_INT = 0x01;
static constexpr uint8_t COLUMN_DATA_TYPE_FLOAT = 0x02;
static constexpr uint8_t COLUMN_DATA_TYPE_VARLEN = 0x03;
static constexpr uint8_t COLUMN_DATA_TYPE_FIXLEN = 0x04;

bool isIntegerType(ColumnDataType type) {
  return ((static_cast<uint16_t>(type) & COLUMN_DATA_TYPE_MASK) >>
             COLUMN_DATA_TYPE_OFFSET) == COLUMN_DATA_TYPE_INT;
}

bool isFloatingType(ColumnDataType type) {
  return ((static_cast<uint16_t>(type) & COLUMN_DATA_TYPE_MASK) >>
             COLUMN_DATA_TYPE_OFFSET) == COLUMN_DATA_TYPE_FLOAT;
}

bool isVarLengthType(ColumnDataType type) {
  return ((static_cast<uint16_t>(type) & COLUMN_DATA_TYPE_MASK) >>
             COLUMN_DATA_TYPE_OFFSET) == COLUMN_DATA_TYPE_VARLEN;
}

bool isFixedLengthType(ColumnDataType type) {
  return ((static_cast<uint16_t>(type) & COLUMN_DATA_TYPE_MASK) >>
             COLUMN_DATA_TYPE_OFFSET) == COLUMN_DATA_TYPE_FIXLEN;
}

Column::Column(string name, ColumnDataType type) : Column(name, type, false) {}

Column::Column(string name, ColumnDataType type, bool null_allowed)
    : name_(name), type_(type), null_allowed_(null_allowed) {
  if (isVarLengthType(type)) {
    string error_msg = ODD::toString(type) + " requires size parameter";
    throw logic_error(error_msg);
  } else if (isFloatingType(type)) {
    string error_msg =
        ODD::toString(type) + " requires precision and scale parameters";
    throw logic_error(error_msg);
  }
}

Column::Column(
    string name, ColumnDataType type, uint8_t size, bool null_allowed)
    : name_(name), type_(type), size_(size), null_allowed_(null_allowed) {
  if (!isVarLengthType(type)) {
    string error_msg = ODD::toString(type) + " does not use the size parameter";
    throw logic_error(error_msg);
  }
}

Column::Column(string name, ColumnDataType type, uint8_t precision,
    uint8_t scale, bool null_allowed)
    : name_(name), type_(type), precision_(precision), scale_(scale),
      null_allowed_(null_allowed) {
  if (!isFloatingType(type)) {
    string error_msg = ODD::toString(type) +
        " does not use the precision and scale parameters";
    throw logic_error(error_msg);
  }
}

string Column::name() { return name_; }

ColumnDataType Column::type() { return type_; }

bool Column::isNullable() { return null_allowed_; }

string Column::toString() {
  if (isVarLengthType(type_)) {
    return name_ + " " + ODD::toString(type_) + "(" + to_string(size_) + ")" +
        (null_allowed_ ? " NULL" : " NOT NULL");
  } else if (isFloatingType(type_)) {
    return name_ + " " + ODD::toString(type_) + "(" + to_string(precision_) +
        "," + to_string(scale_) + ")" + (null_allowed_ ? " NULL" : " NOT NULL");
  } else {
    return name_ + " " + ODD::toString(type_) +
        (null_allowed_ ? " NULL" : " NOT NULL");
  }
}

DatabaseDriver::DatabaseDriver() : DatabaseDriver("PostgreSQL") {}

DatabaseDriver::DatabaseDriver(const string& data_source)
    : db_(make_unique<connection>(data_source)) {}

DatabaseDriver::DatabaseDriver(
    const string& data_source, const string& username, const string& password)
    : db_(make_unique<connection>(data_source, username, password)) {}

void DatabaseDriver::create(const string& table_name, Columns columns) {
  string column_types = "(";
  for (auto column : columns) {
    column_types += column.toString() + ",";
  }
  column_types.pop_back();
  column_types += ")";

  execute("CREATE TABLE ", table_name, column_types);
}

void DatabaseDriver::drop(const string& table_name) {
  execute("DROP TABLE IF EXISTS ", table_name);
}

void DatabaseDriver::insert(const string& table_name,
    vector<string> column_names, DataPoints data_points) {
  string columns = "(";
  string values_placeholder = "(";
  for (auto column : column_names) {
    columns += column + ",";
    values_placeholder += "?,";
  }
  columns.pop_back();
  values_placeholder.pop_back();
  columns += ")";
  values_placeholder += ")";

  auto query = "INSERT INTO " + table_name + " " + columns + " values" +
      values_placeholder + ";";

  prepare(statement, query);
  for (uint32_t i = 0; i < data_points.size(); ++i) {
    // clang-format off
    match(data_points[i], 
        [&](bool value) { statement.bind(i, value); },
        [&](uintmax_t value) { statement.bind(i, value); },
        [&](intmax_t value) { statement.bind(i, value); },
        [&](float value) { statement.bind(i, value); },
        [&](double value) { statement.bind(i, value); },
        [&](string value) { statement.bind(i, value); },
        [&](vector<uint8_t> value) { statement.bind(i, value); });
    // clang-format on
  }

  return execute(statement);
}
} // namespace ODD