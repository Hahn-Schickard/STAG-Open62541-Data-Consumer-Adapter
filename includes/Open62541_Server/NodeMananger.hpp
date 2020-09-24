#ifndef __DCAI_OPEN62541_NODE_MANAGER_HPP_
#define __DCAI_OPEN62541_NODE_MANAGER_HPP_

#include "DataVariant.hpp"
#include "Logger.hpp"
#include "Metric.hpp"
#include "WritableMetric.hpp"

#include <functional>
#include <memory>
#include <open62541/server.h>
#include <optional>
#include <string>
#include <unordered_map>

namespace open62541 {
typedef std::function<Information_Model::DataVariant(void)> ReadCallback;
typedef std::function<void(Information_Model::DataVariant)> WriteCallback;

struct CallbackWrapper {
  const Information_Model::DataType data_type;
  ReadCallback read_callback;
  std::optional<WriteCallback> write_callback;

  CallbackWrapper()
      : data_type(Information_Model::DataType::UNKNOWN), read_callback(nullptr),
        write_callback(nullptr) {}

  CallbackWrapper(Information_Model::DataType type, ReadCallback read)
      : CallbackWrapper(type, read, nullptr) {}

  CallbackWrapper(Information_Model::DataType type, ReadCallback read,
                  WriteCallback write)
      : data_type(type), read_callback(read) {
    if (write == nullptr) {
      write_callback = std::nullopt;
    } else {
      write_callback = write;
    }
  }
};

class NodeManager {
  std::shared_ptr<HaSLL::Logger> logger_;
  std::unordered_map<const UA_NodeId *, std::shared_ptr<CallbackWrapper>>
      node_calbacks_map_;

  /**
   * @brief Searches the node_calbacks_map_ for a given node id
   * Returns an empty shared_ptr if not found.
   *
   * @param node_id
   * @return const CallbackWrapper*
   */
  std::shared_ptr<CallbackWrapper>
  findCallbackWrapper(const UA_NodeId *node_id);

  /**
   * @brief Searches for a given node id within node_calbacks_map_
   * Returns node_calbacks_map_.end() if not found
   *
   * @param nodeId
   * @return std::unordered_map<const UA_NodeId *,
   * std::shared_ptr<CallbackWrapper>>::iterator
   */
  std::unordered_map<const UA_NodeId *,
                     std::shared_ptr<CallbackWrapper>>::iterator
  findIndexPosition(const UA_NodeId *node_id);

public:
  NodeManager();

  UA_StatusCode addNode(Information_Model::DataType type,
                        const UA_NodeId *nodeId, ReadCallback read_callback);
  UA_StatusCode addNode(Information_Model::DataType type,
                        const UA_NodeId *nodeId, ReadCallback read_callback,
                        WriteCallback write_callback);
  UA_StatusCode removeNode(const UA_NodeId *nodeId);

  UA_StatusCode readNodeValue(UA_Server *server, const UA_NodeId *sessionId,
                              void *sessionContext, const UA_NodeId *nodeId,
                              void *nodeContext,
                              UA_Boolean includeSourceTimeStamp,
                              const UA_NumericRange *range,
                              UA_DataValue *value);
  UA_StatusCode writeNodeValue(UA_Server *server, const UA_NodeId *sessionId,
                               void *sessionContext, const UA_NodeId *nodeId,
                               void *nodeContext, const UA_NumericRange *range,
                               const UA_DataValue *value);
};
} // namespace open62541

#endif //__DCAI_OPEN62541_NODE_MANAGER_HPP_