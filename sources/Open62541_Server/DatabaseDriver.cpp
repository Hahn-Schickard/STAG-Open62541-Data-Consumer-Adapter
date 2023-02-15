#include "DatabaseDriver.hpp"
#include "HSCUL/String.hpp"
#include "Variant_Visitor.hpp"

using namespace std;
using namespace nanodbc;

namespace ODD {

string toString(DataType data) {
  string result;
  match(data, // clang-format off
      [&result](bool value) { result = value ? "True" : "False"; },
      [&result](uintmax_t value) { result = to_string(value); },
      [&result](intmax_t value) { result = to_string(value); },
      [&result](float value) { result = to_string(value); },
      [&result](double value) { result = to_string(value); },
      [&result](string value) { result = "\'" + value + "\'"; },
      [&result](vector<uint8_t> value) { result = HSCUL::toString(value); }
      ); // clang-format on
  return result;
}

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

Column::Column(const string& name, ColumnDataType type)
    : Column(name, type, false) {}

Column::Column(const string& name, ColumnDataType type, bool null_allowed)
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
    const string& name, ColumnDataType type, uint8_t size, bool null_allowed)
    : name_(name), type_(type), size_(size), null_allowed_(null_allowed) {
  if (!isVarLengthType(type)) {
    string error_msg = ODD::toString(type) + " does not use the size parameter";
    throw logic_error(error_msg);
  }
}

Column::Column(const string& name, ColumnDataType type, uint8_t precision,
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

string toString(FilterType type) {
  switch (type) {
  case FilterType::EQUAL: {
    return "=";
  }
  case FilterType::NOT_EQUAL: {
    return "!=";
  }
  case FilterType::GREATER: {
    return ">";
  }
  case FilterType::LESS: {
    return "<";
  }
  case FilterType::GREATER_OR_EQUAL: {
    return ">=";
  }
  case FilterType::LESS_OR_EQUAL: {
    return "<=";
  }
  case FilterType::BETWEEN: {
    return "BETWEEN";
  }
  case FilterType::NOT_BETWEEN: {
    return "NOT BETWEEN";
  }
  case FilterType::LIKE: {
    return "LIKE";
  }
  case FilterType::NOT_LIKE: {
    return "NOT LIKE";
  }
  case FilterType::IN: {
    return "IN";
  }
  case FilterType::NOT_IN: {
    return "NOT IN";
  }
  }
}

ColumnValue::ColumnValue(const string& name, DataType value)
    : name_(name), value_(value) {}

string ColumnValue::name() { return name_; }

string ColumnValue::toString() { return name_ + "=" + ODD::toString(value_); }

DataType ColumnValue::value() { return value_; }

ColumnFilter::ColumnFilter(FilterType type, const string column_name)
    : ColumnFilter(type, column_name, DataType{}) {}

ColumnFilter::ColumnFilter(
    FilterType type, const string column_name, DataType value)
    : type_(type), column_(column_name), values_(DataPoints{value}) {
  if (type_ == FilterType::LIKE && !holds_alternative<string>(value)) {
    throw invalid_argument(
        "LIKE filter type requires filter value to be in string format");
  }
}

ColumnFilter::ColumnFilter(
    FilterType type, const string column_name, DataPoints values)
    : type_(type), column_(column_name), values_(values_) {
  if ((type_ == FilterType::BETWEEN || type_ == FilterType::NOT_BETWEEN) &&
      values_.size() != 2) {
    throw invalid_argument(
        "BETWEEN filter type requires two data points as filter value");
  }
  if ((type_ == FilterType::BETWEEN || type_ == FilterType::NOT_BETWEEN) ||
      (type_ != FilterType::IN || type_ != FilterType::NOT_IN)) {
    throw invalid_argument("Only BETWEEN, NOT BETWEEN, IN and NOT IN filter "
                           "types support multiple values");
  }
}

FilterType ColumnFilter::type() { return type_; }

string ColumnFilter::toString() {
  string filter_value;
  if (type_ == FilterType::BETWEEN) {
    filter_value =
        ODD::toString(values_[0]) + " AND " + ODD::toString(values_[1]);
  } else if (type_ == FilterType::IN) {
    filter_value = "(";
    for (auto value : values_) {
      filter_value += ODD::toString(value) + ",";
    }
    filter_value.pop_back();
    filter_value += ")";
  } else {
    filter_value = ODD::toString(values_[0]);
  }
  return "WHERE " + column_ + ODD::toString(type_) + filter_value;
}

DataPoints ColumnFilter::values() { return values_; }

string ColumnFilter::column() { return column_; }

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

void DatabaseDriver::insert(
    const string& table_name, vector<ColumnValue> values) {
  string column_names = "(";
  string column_values = "(";
  for (auto value : values) {
    column_names += value.name() + ",";
    column_values += ODD::toString(value.value()) + ",";
  }
  column_names.pop_back();
  column_values.pop_back();
  column_names += ")";
  column_values += ")";

  auto query = "INSERT INTO " + table_name + " " + column_names + " values" +
      column_values + ";";

  execute(query);
}

  nanodbc::execute(statement);
}
} // namespace ODD