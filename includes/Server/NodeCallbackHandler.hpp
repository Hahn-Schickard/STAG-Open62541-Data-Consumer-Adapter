#ifndef __DCAI_OPEN62541_NODE_MANAGER_HPP_
#define __DCAI_OPEN62541_NODE_MANAGER_HPP_

#include "NodeId.hpp"
#include "Open62541Server.hpp"

#include <Information_Model/Readable.hpp>
#include <Information_Model/Writable.hpp>
#include <open62541/plugin/log.h>
#include <open62541/server.h>
#include <unordered_map> // replace with threadsafe version

#include <functional>
#include <memory>
#include <optional>
#include <string>

template <> class std::hash<open62541::NodeId> {
public:
  size_t operator()(const open62541::NodeId& node_id) const {
    return UA_NodeId_hash(&node_id.base());
  }
};

namespace open62541 {
struct CallbackWrapper {
  using ReadCallback = std::function<Information_Model::DataVariant()>;
  using WriteCallback =
      std::function<void(const Information_Model::DataVariant&)>;
  using ExecuteCallback =
      std::function<void(const Information_Model::Parameters&)>;
  using CallCallback = std::function<Information_Model::DataVariant(
      const Information_Model::Parameters&)>;
  CallbackWrapper() = default;
  CallbackWrapper(Information_Model::DataType type, ReadCallback read_callback);
  CallbackWrapper(
      Information_Model::DataType type, WriteCallback write_callback);
  CallbackWrapper(Information_Model::DataType type, ReadCallback read_callback,
      WriteCallback write_callback);
  CallbackWrapper(Information_Model::DataType type,
      const Information_Model::ParameterTypes& parameters,
      ExecuteCallback execute_callback);
  CallbackWrapper(Information_Model::DataType type,
      const Information_Model::ParameterTypes& parameters,
      CallCallback call_callback);

  Information_Model::DataType data_type_ = Information_Model::DataType::Unknown;
  Information_Model::ParameterTypes parameters_;
  ReadCallback readable_ = nullptr;
  WriteCallback writable_ = nullptr;
  ExecuteCallback executable_ = nullptr;
  CallCallback callable_ = nullptr;
};
using CallbackWrapperPtr = std::shared_ptr<CallbackWrapper>;

class NodeCallbackHandler {
  using CallbackMap = std::unordered_map<NodeId,
      CallbackWrapperPtr>; // use threadsafe alternative
  static CallbackMap callbacks_;
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
   * The caller is responsible for calling `UA_DataValue_clear` on `value`
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

  static UA_StatusCode callNodeMethod(UA_Server* server,
      const UA_NodeId* session_id, void* session_context,
      const UA_NodeId* method_id, void* method_context,
      const UA_NodeId* object_id, void* object_context, size_t input_size,
      const UA_Variant* input, size_t output_size, UA_Variant* output);
};
} // namespace open62541

#endif //__DCAI_OPEN62541_NODE_MANAGER_HPP_