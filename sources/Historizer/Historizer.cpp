#include "Historizer.hpp"
#include "Exceptions.hpp"
#include "HistorizerUtils.hpp"
#include "StringConverter.hpp"
#include "UaVariantOperators.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <open62541/client_subscriptions.h>
#include <open62541/server.h>

#include <string>

namespace open62541 {
using namespace std;
using namespace HaSLL;
using namespace pqxx;

struct NoHistorizerInContext : runtime_error {
  NoHistorizerInContext() : runtime_error("Historizer is not set in context") {}
};

struct BadContinuationPoint : runtime_error {
  BadContinuationPoint() : runtime_error("Corrupted Continuation Point") {}
};

struct NoData : runtime_error {
  NoData() : runtime_error("No data") {}
};

struct NoBoundData : runtime_error {
  NoBoundData() : runtime_error("No bound data") {}
};

pqxx::connection connect() {
  return connection("service=stag_open62541_historizer");
}

bool checkTable(const string& name) {
  auto session = connect();
  work transaction(session);
  auto result = transaction.exec(
      "SELECT 1 FROM information_schema.tables WHERE table_name = " + name);
  return !result.empty();
}

bool isHistorized(const UA_NodeId* node_id) {
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

Historizer* getHistorizer(void* context_ptr) {
  if (context_ptr == nullptr) {
    throw NoHistorizerInContext();
  }

  auto* historizer = static_cast<Historizer*>(context_ptr);
  if (historizer == nullptr) {
    throw NoHistorizerInContext();
  }
  return historizer;
}

void dataChangedCallback(UA_Server*, UA_UInt32, void* monitored_item_context,
    const UA_NodeId* node_id, void*, UA_UInt32 attribute_id,
    const UA_DataValue* value) {
  try {
    auto* historizer = getHistorizer(monitored_item_context);
    historizer->dataChanged(node_id, attribute_id, value);
  } catch (...) {
    // suppress any exceptions
  }
}

void clearCallback(UA_HistoryDatabase* database) {
  if (database == nullptr || database->context == nullptr) {
    return;
  } else {
    // database destruction is handled by the adapter, not open62541 server
    database->context = nullptr;
  }
}

void setValueCallback(UA_Server*, void* hdb_context, const UA_NodeId*, void*,
    const UA_NodeId* node_id, UA_Boolean historizing,
    const UA_DataValue* value) {
  try {
    auto* historizer = getHistorizer(hdb_context);
    historizer->write(node_id, historizing, value);
  } catch (...) {
    // suppress any exceptions
  }
}

void readRawCallback(UA_Server*, void* hdb_context, const UA_NodeId*, void*,
    const UA_RequestHeader* request_header,
    const UA_ReadRawModifiedDetails* history_read_details,
    UA_TimestampsToReturn timestamps_to_return,
    UA_Boolean release_continuation_points, size_t nodes_to_read_size,
    const UA_HistoryReadValueId* nodes_to_read,
    UA_HistoryReadResponse* response,
    UA_HistoryData* const* const history_data) {
  try {
    auto* historizer = getHistorizer(hdb_context);
    historizer->readRaw(request_header, history_read_details,
        timestamps_to_return, release_continuation_points, nodes_to_read_size,
        nodes_to_read, response, history_data);
  } catch (...) {
    response->resultsSize = 1;
    response->results[0].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
  }
}

void readAtTimeCallback(UA_Server*, void* hdb_context, const UA_NodeId*, void*,
    const UA_RequestHeader* request_header,
    const UA_ReadAtTimeDetails* history_read_details,
    UA_TimestampsToReturn timestamps_to_return,
    UA_Boolean release_continuation_points, size_t nodes_to_read_size,
    const UA_HistoryReadValueId* nodes_to_read,
    UA_HistoryReadResponse* response,
    UA_HistoryData* const* const history_data) {
  try {
    auto* historizer = getHistorizer(hdb_context);
    historizer->readAtTime(request_header, history_read_details,
        timestamps_to_return, release_continuation_points, nodes_to_read_size,
        nodes_to_read, response, history_data);
  } catch (...) {
    response->resultsSize = 1;
    response->results[0].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
  }
}

Historizer::Historizer() : logger_(LoggerManager::registerTypedLogger(this)) {
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

void Historizer::dataChanged(const UA_NodeId* node_id, UA_UInt32 attribute_id,
    const UA_DataValue* value) {
  UA_Boolean historize = false;
  if ((attribute_id & UA_ATTRIBUTEID_HISTORIZING) != 0) {
    historize = true;
  }

  write(node_id, historize, value);
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
    auto value_type = toSqlType(type);
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
    auto* monitored_item_context = static_cast<void*>(this);
    auto result = UA_Server_createDataChangeMonitoredItem(server,
        UA_TIMESTAMPSTORETURN_BOTH, monitor_request, monitored_item_context,
        &dataChangedCallback);
    return result.statusCode;
  } catch (const exception& ex) {
    logger_->critical(
        "An unhandled exception occurred while trying to register {} node. "
        "Exception: {}",
        target, ex.what());
    return UA_STATUSCODE_BADUNEXPECTEDERROR;
  }
}

void Historizer::write(const UA_NodeId* node_id, UA_Boolean historizing,
    const UA_DataValue* value) {
  auto target = toString(node_id);
  if (!historizing) {
    logger_->info("Node {} is not configured for historization ", target);
    return;
  }

  if (value == nullptr) {
    logger_->error(
        "Failed to historize Node {} value. No data provided.", target);
    return;
  }

  try {
    auto session = connect();
    work transaction(session);

    string source_time;
    if (value->hasSourceTimestamp) {
      source_time = toString(value->sourceTimestamp);
    }
    string server_time;
    if (value->hasServerTimestamp) {
      server_time = toString(value->serverTimestamp);
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
  } catch (exception& ex) {
    logger_->error("Failed to historize Node {} value due to an exception. "
                   "Exception: {}",
        toString(node_id), ex.what());
  }
}

HistoryResults Historizer::readHistory(
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
  auto results = makeHistoryResults(rows, timestamps_to_return, type_map_);

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

void Historizer::readRaw(const UA_RequestHeader* request_header,
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

UA_StatusCode Historizer::readAndAppendHistory(
    const UA_ReadAtTimeDetails* history_read_details,
    UA_UInt32 /*timeout_hint*/, UA_TimestampsToReturn timestamps_to_return,
    UA_NodeId node_id, const UA_ByteString* /*continuation_point_in*/,
    [[maybe_unused]] UA_ByteString* continuation_point_out,
    UA_HistoryData* history_data) {

  auto columns = setColumnNames(timestamps_to_return);
  UA_StatusCode status = UA_STATUSCODE_GOOD;
  using namespace HistorianBits;
  HistoryResults results;
  for (size_t i = 0; i < history_read_details->reqTimesSize; ++i) {
    auto timestamp = toString(history_read_details->reqTimes[i]);
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
      auto nearest_before = makeHistoryResult(
          transaction.exec(nearest_query, params{"<"}).expect_rows(1).at(0),
          timestamps_to_return, type_map_);
      auto nearest_after = makeHistoryResult(
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
      auto raw = makeHistoryResults(rows, timestamps_to_return, type_map_);
      results.insert(results.end(), raw.begin(), raw.end());
      setHistorianBits(&status, DataLocation::Raw);
    }
  }
  expandHistoryResult(history_data, results);
  return status;
}

void Historizer::readAtTime(const UA_RequestHeader* request_header,
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
      } catch (const OutOfMemory& ex) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
      } catch (const runtime_error& ex) {
        // handle unexpected read exceptions here
        response->results[i].statusCode = UA_STATUSCODE_BADUNEXPECTEDERROR;
      }
    }
  }
}

UA_HistoryDatabase createDatabaseStruct(const HistorizerPtr& historizer) {
  UA_HistoryDatabase database;
  memset(&database, 0, sizeof(UA_HistoryDatabase));

  database.context = historizer.get();
  database.clear = &clearCallback;
  database.setValue = &setValueCallback;
  database.setEvent = nullptr;
  database.readRaw = &readRawCallback;
  database.readModified = nullptr;
  database.readEvent = nullptr;
  database.readProcessed = nullptr;
  database.readAtTime = &readAtTimeCallback;
  database.updateData = nullptr;
  database.deleteRawModified = nullptr;

  return database;
}

} // namespace open62541
