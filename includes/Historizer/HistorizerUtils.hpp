#ifndef __OPEN62541_HISTORIZER_UTILS_HPP
#define __OPEN62541_HISTORIZER_UTILS_HPP

#include "HistoryResult.hpp"

#include <open62541/types.h>
#include <open62541/types_generated.h>
#include <pqxx/pqxx>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace open62541 {

void createDomainRestrictions(pqxx::connection session);

using TypeMap = std::unordered_map<int64_t, UA_DataTypeKind>;

TypeMap queryTypeOIDs(pqxx::connection session);

std::string getCurrentTimestamp();

std::string toSanitizedString(const UA_NodeId* node_id);

void addNodeValue(pqxx::params* values, const UA_Variant* variant);

std::string setColumnNames(UA_TimestampsToReturn timestamps_to_return);

std::string setColumnFilters(
    UA_Boolean include_bounds, UA_DateTime start_time, UA_DateTime end_time);

std::string setColumnFilters(UA_Boolean include_bounds, UA_DateTime start_time,
    UA_DateTime end_time, const UA_ByteString* continuation_point);

UA_DateTime toUaDateTime(const std::string& data);

UA_Variant toUaVariant(const pqxx::field& data, const TypeMap& type_map);

UA_ByteString* makeContinuationPoint(intmax_t last_index);

HistoryResult makeHistoryResult(const pqxx::row& entry,
    UA_TimestampsToReturn timestamps_to_return, const TypeMap& type_map);

HistoryResults makeHistoryResults(const pqxx::result& rows,
    UA_TimestampsToReturn timestamps_to_return, const TypeMap& type_map);

HistoryResult interpolateValues(UA_DateTime target_timestamp,
    const HistoryResult& before, const HistoryResult& after);

UA_StatusCode appendUADataValue(
    UA_HistoryData* result, UA_DataValue* data_points, size_t data_points_size);

UA_StatusCode expandHistoryResult(
    UA_HistoryData* result, const HistoryResults& rows);
} // namespace open62541
#endif //__OPEN62541_HISTORIZER_UTILS_HPP