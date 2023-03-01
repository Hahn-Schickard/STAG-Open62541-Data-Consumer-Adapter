#include "DatabaseDriver.hpp"
#include "HSCUL/String.hpp"
#include "SQL_DataTypes.h"
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
  case ColumnDataType::TEXT: {
    return "TEXT";
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
    : Column(name, type, ColumnModifier::NOT_NULL) {}

Column::Column(
    const string& name, ColumnDataType type, ColumnModifier modifier) // NOLINT
    : name_(name), type_(type), modifier_(modifier) { // NOLINT
  if (isVarLengthType(type)) {
    string error_msg = ODD::toString(type) + " requires size parameter";
    throw logic_error(error_msg);
  } else if (isFloatingType(type)) {
    string error_msg =
        ODD::toString(type) + " requires precision and scale parameters";
    throw logic_error(error_msg);
  }
}

Column::Column(const string& name, ColumnDataType type, uint8_t size, // NOLINT
    ColumnModifier modifier)
    : name_(name), type_(type), size_(size), // NOLINT
      modifier_(modifier) {
  if (!isVarLengthType(type)) {
    string error_msg = ODD::toString(type) + " does not use the size parameter";
    throw logic_error(error_msg);
  }
}

Column::Column(const string& name, ColumnDataType type, // NOLINT
    uint8_t precision, uint8_t scale, ColumnModifier modifier)
    : name_(name), type_(type), precision_(precision), scale_(scale), // NOLINT
      modifier_(modifier) {
  if (!isFloatingType(type)) {
    string error_msg = ODD::toString(type) +
        " does not use the precision and scale parameters";
    throw logic_error(error_msg);
  }
}

string Column::name() { return name_; }

ColumnDataType Column::type() { return type_; }

bool Column::isNullable() { return modifier_ == ColumnModifier::NULL_ALLOWED; }

string toString(ColumnModifier modifier) {
  switch (modifier) {
  case ColumnModifier::NULL_ALLOWED: {
    return " NULL";
  }
  case ColumnModifier::AUTO_INCREMENT: {
    return " NOT NULL GENERATED ALWAYS AS IDENTITY";
  }
  default: { [[fallthrough]]; }
  case ColumnModifier::NOT_NULL: {
    return " NOT NULL";
  }
  }
}

string Column::toString() {
  if (isVarLengthType(type_)) {
    return name_ + " " + ODD::toString(type_) + "(" + to_string(size_) + ")" +
        ODD::toString(modifier_);
  } else if (isFloatingType(type_)) {
    return name_ + " " + ODD::toString(type_) + "(" + to_string(precision_) +
        "," + to_string(scale_) + ")" + ODD::toString(modifier_);
  } else {
    return name_ + " " + ODD::toString(type_) + ODD::toString(modifier_);
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
  default: { return ""; }
  }
}

ColumnValue::ColumnValue(const string& name, DataType value) // NOLINT
    : name_(name), value_(value) {} // NOLINT

string ColumnValue::name() { return name_; }

string ColumnValue::toString() { return name_ + "=" + ODD::toString(value_); }

DataType ColumnValue::value() { return value_; }

ColumnFilter::ColumnFilter(FilterType type, const string& column_name)
    : ColumnFilter(type, column_name, DataType{}) {}

ColumnFilter::ColumnFilter(
    FilterType type, const string& column_name, DataType value) // NOLINT
    : type_(type), column_(column_name), values_(DataPoints{value}) { // NOLINT
  if (type_ == FilterType::LIKE && !holds_alternative<string>(value)) {
    throw invalid_argument(
        "LIKE filter type requires filter value to be in string format");
  }
}

ColumnFilter::ColumnFilter(
    FilterType type, const string& column_name, DataPoints values) // NOLINT
    : type_(type), column_(column_name), values_(values) { // NOLINT
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
    for (const auto& value : values_) {
      filter_value += ODD::toString(value) + ",";
    }
    filter_value.pop_back();
    filter_value += ")";
  } else {
    filter_value = ODD::toString(values_[0]);
  }
  return column_ + ODD::toString(type_) + filter_value;
}

DataPoints ColumnFilter::values() { return values_; }

string ColumnFilter::column() { return column_; }

bool OverrunPoint::hasMoreValues() { return !columns_.empty(); }

vector<ColumnValue> OverrunPoint::getOverrunRecord() { return columns_; }

DatabaseDriver::DatabaseDriver() : DatabaseDriver("PostgreSQL") {}

DatabaseDriver::DatabaseDriver(const string& data_source)
    : DatabaseDriver(data_source, "", "") {}

DatabaseDriver::DatabaseDriver(
    const string& data_source, const string& username, const string& password)
    : db_(make_unique<connection>(data_source, username, password)) {}

void DatabaseDriver::create(
    const string& table_name, Columns columns, bool tmp) {
  string column_types = "(";
  for (auto column : columns) {
    column_types += column.toString() + ",";
  }
  column_types.pop_back();
  column_types += ")";

  string query = "CREATE TABLE ";

  if (tmp) {
    query += "IF NOT EXISTS ";
  }

  execute(query, table_name, column_types);
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

void DatabaseDriver::update(const string& table_name,
    vector<ColumnFilter> filters, vector<ColumnValue> values) {
  string filter_values;
  for (auto filter : filters) {
    filter_values += filter.toString() + " AND ";
  }
  filter_values.erase(filter_values.rfind(" AND "));

  string column_values;
  for (auto value : values) {
    column_values += value.toString() + ",";
  }
  column_values.pop_back();
  execute(
      "UPDATE ", table_name, " SET ", column_values, " WHERE ", filter_values);
}

void DatabaseDriver::update(
    const string& table_name, vector<ColumnFilter> filters, ColumnValue value) {
  update(table_name, move(filters), vector<ColumnValue>{move(value)});
}

void DatabaseDriver::update(
    const string& table_name, ColumnFilter filter, vector<ColumnValue> values) {
  update(table_name, vector<ColumnFilter>{move(filter)}, move(values));
}

void DatabaseDriver::update(
    const string& table_name, ColumnFilter filter, ColumnValue value) {
  update(table_name, vector<ColumnFilter>{move(filter)},
      vector<ColumnValue>{move(value)});
}

DataType intoDataType(
    int type_id, const short column_id, nanodbc::result& data) {
  switch (type_id) {
  case SQL_C_CHAR: {
    return DataType(data.get<string>(column_id));
  }
  case SQL_C_UTINYINT:
    [[fallthrough]];
  case SQL_C_TINYINT:
    [[fallthrough]];
  case SQL_C_USHORT:
    [[fallthrough]];
  case SQL_C_ULONG:
    [[fallthrough]];
  case SQL_C_UBIGINT: {
    return DataType(data.get<uintmax_t>(column_id));
  }
  case SQL_C_STINYINT:
    [[fallthrough]];
  case SQL_C_SSHORT:
    [[fallthrough]];
  case SQL_C_SHORT:
    [[fallthrough]];
  case SQL_C_LONG:
    [[fallthrough]];
  case SQL_C_SLONG:
    [[fallthrough]];
  case SQL_C_SBIGINT: {
    return DataType(data.get<intmax_t>(column_id));
  }
  case SQL_C_FLOAT: {
    return DataType(data.get<float>(column_id));
  }
  case SQL_C_DOUBLE: {
    return DataType(data.get<double>(column_id));
  }
  case SQL_C_TYPE_DATE:
    [[fallthrough]];
  case SQL_C_DATE: {
    auto date = data.get<nanodbc::date>(column_id);
    string value = to_string(date.year) + "-" + to_string(date.month) + "-" +
        to_string(date.day);
    return DataType(value);
  }
  case SQL_C_TYPE_TIME:
    [[fallthrough]];
  case SQL_C_TIME: {
    auto time = data.get<nanodbc::time>(column_id);
    string value = to_string(time.hour) + ":" + to_string(time.min) + ":" +
        to_string(time.sec);
    return DataType(value);
  }
  case SQL_C_TYPE_TIMESTAMP:
    [[fallthrough]];
  case SQL_C_TIMESTAMP: {
    auto timestamp = data.get<nanodbc::timestamp>(column_id);
    string value = to_string(timestamp.year) + "-" +
        to_string(timestamp.month) + "-" + to_string(timestamp.day) + " " +
        to_string(timestamp.hour) + ":" + to_string(timestamp.min) + ":" +
        to_string(timestamp.sec) + "." + to_string(timestamp.fract);
    return DataType(value);
  }
  case SQL_C_BINARY: {
    return DataType(data.get<vector<uint8_t>>(column_id));
  }
  case SQL_C_NUMERIC:
    [[fallthrough]];
  default: {
    string error_msg = "Requested for DataType conversion for an unsupported "
                       "SQL C Type: " +
        to_string(type_id);
    throw domain_error(error_msg);
  }
  }
}

vector<ColumnValue> intoColumnValues(nanodbc::result data) {
  vector<ColumnValue> result;
  for (short column = 0; column < data.columns(); ++column) {
    result.emplace_back(data.column_name(column),
        intoDataType(data.column_c_datatype(column), column, data));
  }
  return result;
}

unordered_map<size_t, vector<ColumnValue>> intoRowValues(nanodbc::result data) {
  unordered_map<size_t, vector<ColumnValue>> result;
  size_t row = 0;
  do {
    vector<ColumnValue> values = intoColumnValues(data);
    result.emplace(row, values);
    ++row;
  } while (data.next());
  return result;
}

unordered_map<size_t, vector<ColumnValue>> DatabaseDriver::select(
    const string& table_name, vector<string> column_names,
    vector<ColumnFilter> filters, optional<size_t> response_limit,
    const string& order_by_column, bool highest_value_first,
    OverrunPoint* overrun) {
  string columns;
  for (const auto& column_name : column_names) {
    columns += column_name + ",";
  }
  columns.pop_back();

  auto query = "SELECT " + columns + " FROM " + table_name;
  if (!filters.empty()) {
    string filter_values;
    for (auto filter : filters) {
      filter_values += filter.toString() + " AND ";
    }
    filter_values.erase(filter_values.rfind(" AND "));

    query += " WHERE " + filter_values;

    if (response_limit.has_value()) {
      auto record_limit = response_limit.value();
      if (overrun != nullptr) {
        // get the number of records that match the given filter parameters
        auto record_count = execute(
            "SELECT COUNT(*) FROM ", table_name, " WHERE ", filter_values)
                                .get<size_t>(0);
        if (record_count > record_limit) {
          // if there are more values, that could be fetched, get the row
          // values, from which to fetch the next point
          auto last_row = execute("SELECT * FROM ", table_name, " WHERE ",
              filter_values, " ORDER BY ", order_by_column,
              (highest_value_first ? " DESC" : " ASC"), "OFFSET ",
              to_string(record_limit - 1), " ROWS FETCH NEXT 1 ROWS ONLY");
          overrun->columns_ = intoColumnValues(last_row);
        }
      }
      query += " ORDER BY " + order_by_column +
          (highest_value_first ? " DESC" : " ASC") + " FETCH FIRST " +
          to_string(record_limit) + " ROWS ONLY";
    }
  }
  auto result = execute(query);
  return intoRowValues(result);
}

unordered_map<size_t, vector<ColumnValue>> DatabaseDriver::select(
    const string& table_name, vector<string> column_names,
    optional<size_t> response_limit, const string& order_by_column,
    bool highest_value_first, OverrunPoint* overrun) {
  return select(table_name, move(column_names), vector<ColumnFilter>{},
      response_limit, order_by_column, highest_value_first, overrun);
}

unordered_map<size_t, vector<ColumnValue>> DatabaseDriver::select(
    const string& table_name, vector<string> column_names, ColumnFilter filter,
    optional<size_t> response_limit, const string& order_by_column,
    bool highest_value_first, OverrunPoint* overrun) {
  return select(table_name, move(column_names),
      vector<ColumnFilter>{move(filter)}, response_limit, order_by_column,
      highest_value_first, overrun);
}

unordered_map<size_t, vector<ColumnValue>> DatabaseDriver::select(
    const string& table_name, vector<ColumnFilter> filter,
    optional<size_t> response_limit, const string& order_by_column,
    bool highest_value_first, OverrunPoint* overrun) {
  return select(table_name, vector<string>{"*"}, move(filter), response_limit,
      order_by_column, highest_value_first, overrun);
}

unordered_map<size_t, vector<ColumnValue>> DatabaseDriver::select(
    const string& table_name, ColumnFilter filter,
    optional<size_t> response_limit, const string& order_by_column,
    bool highest_value_first, OverrunPoint* overrun) {
  return select(table_name, vector<ColumnFilter>{move(filter)}, response_limit,
      order_by_column, highest_value_first, overrun);
}

unordered_map<size_t, vector<ColumnValue>> DatabaseDriver::select(
    const string& table_name, string column_name,
    optional<size_t> response_limit, const string& order_by_column,
    bool highest_value_first, OverrunPoint* overrun) {
  return select(table_name, vector<string>{move(column_name)}, response_limit,
      order_by_column, highest_value_first, overrun);
}
} // namespace ODD