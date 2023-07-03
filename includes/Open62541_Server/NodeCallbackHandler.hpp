#ifndef __DCAI_OPEN62541_NODE_MANAGER_HPP_
#define __DCAI_OPEN62541_NODE_MANAGER_HPP_

#include "Information_Model/Metric.hpp"
#include "Information_Model/WritableMetric.hpp"
#include "Open62541Server.hpp"

#include "open62541/plugin/log.h"
#include <functional>
#include <memory>
#include <open62541/server.h>
#include <optional>
#include <string>
#include <unordered_map>

namespace open62541 {
struct CallbackWrapper {
  using ReadCallback = std::function<Information_Model::DataVariant()>;
  using WriteCallback = std::function<void(Information_Model::DataVariant)>;
  using ExecuteCallback =
      std::function<void(Information_Model::Function::Parameters)>;
  using CallCallback = std::function<Information_Model::DataVariant(
      Information_Model::Function::Parameters)>;

  const Information_Model::DataType data_type_ =
      Information_Model::DataType::UNKNOWN;
  const ReadCallback readable_ = nullptr;
  const WriteCallback writable_ = nullptr;
  const ExecuteCallback executable_ = nullptr;
  const CallCallback callable_ = nullptr;

  CallbackWrapper() = default;
  CallbackWrapper(
      Information_Model::DataType type, const ReadCallback& read_callback);
  CallbackWrapper(
      Information_Model::DataType type, const WriteCallback& write_callback);
  CallbackWrapper(Information_Model::DataType type,
      const ReadCallback& read_callback, const WriteCallback& write_callback);
  CallbackWrapper(Information_Model::DataType type,
      const ExecuteCallback& execute_callback);
  CallbackWrapper(Information_Model::DataType type,
      const ExecuteCallback& execute_callback,
      const CallCallback& cancel_callback);
};
using CallbackWrapperPtr = std::shared_ptr<CallbackWrapper>;

struct UA_NodeId_Hasher {
  size_t operator()(UA_NodeId const& node_id) const noexcept {
    return UA_NodeId_hash(&node_id);
  }
};

class NodeCallbackHandler {
  using NodeCalbackMap =
      std::unordered_map<UA_NodeId, CallbackWrapperPtr, UA_NodeId_Hasher>;
  static NodeCalbackMap node_calbacks_map_;
  // Invariant: No CallbackWrapperPtr is empty
  static const UA_Logger* logger_;

  static CallbackWrapperPtr findCallbackWrapper(const UA_NodeId* node_id);

public:
  /**
   * @brief Initialises NodeCallbackHandler with a given open62541 logger
   * pointer
   *
   * @param logger
   */
  static void initialise(const UA_Logger* logger);
  /**
   * @brief Cleans up the callback map
   *
   */
  static void destroy();

  /**
   * @pre There is no callback bound to nodeId.
   * @pre callback_wrapper is non-empty.
   */
  static UA_StatusCode addNodeCallbacks(
      UA_NodeId node_id, const CallbackWrapperPtr& callback_wrapper);

  /**
   * @pre There is a callback bound to nodeId.
   */
  static UA_StatusCode removeNodeCallbacks(const UA_NodeId* node_id);

  /**
   * @brief Read callback used by OPC UA Clients to read the current value of a
   * specific node
   *
   * @param server - unused
   * @param sessionId - unused
   * @param sessionContext - unused
   * @param nodeId
   * @param nodeContext - unused
   * @param includeSourceTimeStamp - unused
   * @param range - unused
   * @param value
   * @return UA_StatusCode
   */
  static UA_StatusCode readNodeValue(UA_Server* server,
      const UA_NodeId* session_id, void* session_context,
      const UA_NodeId* node_id, void* node_context,
      UA_Boolean include_source_time_stamp, const UA_NumericRange* range,
      UA_DataValue* value);

  /**
   * @brief Write callback used by OPC UA Clients to write a specified value to
   * a specific node
   *
   * @param server - unused
   * @param sessionId - unused
   * @param sessionContext - unused
   * @param nodeId
   * @param nodeContext - unused
   * @param range - unused
   * @param value
   * @return UA_StatusCode
   */
  static UA_StatusCode writeNodeValue(UA_Server* server,
      const UA_NodeId* session_id, void* session_context,
      const UA_NodeId* node_id, void* node_context,
      const UA_NumericRange* range, const UA_DataValue* value);
};
} // namespace open62541

#endif //__DCAI_OPEN62541_NODE_MANAGER_HPP_