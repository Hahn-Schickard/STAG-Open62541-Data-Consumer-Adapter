#ifndef __OPCUA_HISTORIZER_HPP
#define __OPCUA_HISTORIZER_HPP
#ifdef UA_ENABLE_HISTORIZING
#include <HaSLL/Logger.hpp>
#include <open62541/plugin/historydatabase.h>
#include <soci/soci.h>

#include <filesystem>

namespace open62541 {
struct Historizer {

  /**
   * @brief Create default historizer object with default OODD::DatabaseDriver
   * values
   *
   */
  Historizer(const filesystem::path& config);

  ~Historizer();
  /**
   * @brief Create UA_MonitoredItem for a given node ide and use the data change
   * callback to as a historization call for the database
   *
   * @see
   * https://github.com/open62541/open62541/blob/master/plugins/historydata/ua_history_data_gathering_default.c#L59
   *
   */
  static UA_StatusCode registerNodeId(
      UA_Server* server, UA_NodeId node_id, const UA_DataType* type);

  /**
   * @brief UA_HistoryDatabase constructor
   *
   * @return UA_HistoryDatabase
   */
  static UA_HistoryDatabase createDatabase();

private:
  /**
   * @brief Used as destructor for the UA_HistoryDatabase struct by UA_Server
   * instance
   *
   * @param database
   */
  static void clear(UA_HistoryDatabase* database);

  static bool checkTable(const std::string& name);

  static std::string getIncrementedPrimaryKey();

  static bool isHistorized(const UA_NodeId* node_id);

  /**
   * @brief Called when a given node receives new data value.
   *
   * @param server - not used
   * @param hdb_context - not used
   * @param session_id - used to identify the session the value is set for
   * @param session_context - used to get session context if needed
   * @param node_id - target node id for which the new data value is set
   * @param historizing - nodes historization setting flag, if it false the
   * new value is ignored
   * @param value - new node value to historize
   */
  static void setValue(UA_Server* server, void* hdb_context,
      const UA_NodeId* session_id, void* session_context,
      const UA_NodeId* node_id, UA_Boolean historizing,
      const UA_DataValue* value);

  /**
   * @brief Callback implementation for monitored item data change call
   *
   * @param server - the parent server, that node exists on
   * @param monitoredItemId - unkown
   * @param monitoredItemContext - not used
   * @param node_id - node id, the new value is set for
   * @param nodeContext - not used
   * @param attributeId - unkown
   * @param value - new node value
   */
  static void dataChanged(UA_Server* server, UA_UInt32 monitored_item_id,
      void* monitored_item_context, const UA_NodeId* node_id,
      void* node_context, UA_UInt32 attribute_id, const UA_DataValue* value);

  static std::vector<std::string> readHistory(
      const UA_ReadRawModifiedDetails* history_read_details,
      UA_UInt32 timeout_hint, UA_TimestampsToReturn timestamps_to_return,
      UA_NodeId node_id, const UA_ByteString* continuation_point_in,
      UA_ByteString* continuation_point_out);

  /**
   * @brief Called by UA_Server instance when a history read is requested with
   * isRawReadModified set to false
   *
   * @param server - not used
   * @param hdb_context - not used
   * @param session_id -  not used
   * @param session_context -  not used
   * @param request_header -  not used
   * @param history_read_details - specifies which values to return
   * @param timestamps_to_return - specifies how to format the result
   * @param release_continuation_points - if true, returns no historic values
   * @param nodes_to_read_size - how many node ids to read
   * @param nodes_to_read - array of node ids to read
   * @param response - used to indicate request success/failure
   * @param history_data - history result array
   */
  static void readRaw(UA_Server* server, void* hdb_context,
      const UA_NodeId* session_id, void* session_context,
      const UA_RequestHeader* request_header,
      const UA_ReadRawModifiedDetails* history_read_details,
      UA_TimestampsToReturn timestamps_to_return,
      UA_Boolean release_continuation_points, size_t nodes_to_read_size,
      const UA_HistoryReadValueId* nodes_to_read,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const history_data);

  static UA_StatusCode readAndAppendHistory(
      const UA_ReadAtTimeDetails* history_read_details, UA_UInt32 timeout_hint,
      UA_TimestampsToReturn timestamps_to_return, UA_NodeId node_id,
      const UA_ByteString* continuation_point_in,
      UA_ByteString* continuation_point_out, UA_HistoryData*);
  /**
   * @brief Not documented and not implemented by open62541
   *
   * @param server - the parent server, that node exists on
   * @param hdb_context - not used
   * @param session_id - used to identify the session
   * @param session_context - used to get session context if needed
   * @param request_header - ua client request header
   * @param history_read_details - specifies how to format the read result
   * @param timestamps_to_return - specifies which timestamps to return
   * @param release_continuation_points - unknown @todo: figure this out
   * @param nodes_to_read_size - how many node ids to read
   * @param nodes_to_read - array of node ids to read
   * @param response - used to indicate request failure
   * @param history_data - history result
   */
  static void readAtTime(UA_Server* server, void* hdb_context,
      const UA_NodeId* session_id, void* session_context,
      const UA_RequestHeader* request_header,
      const UA_ReadAtTimeDetails* history_read_details,
      UA_TimestampsToReturn timestamps_to_return,
      UA_Boolean release_continuation_points, size_t nodes_to_read_size,
      const UA_HistoryReadValueId* nodes_to_read,
      UA_HistoryReadResponse* response,
      UA_HistoryData* const* const history_data);

  template <typename... Types>
  static void log(
      HaSLL::SeverityLevel level, std::string message, Types... args);

  static HaSLL::LoggerPtr logger_;
  static soci::session db_;
};
} // namespace open62541
#endif // UA_ENABLE_HISTORIZING
#endif // __OPCUA_HISTORIZER_HPP