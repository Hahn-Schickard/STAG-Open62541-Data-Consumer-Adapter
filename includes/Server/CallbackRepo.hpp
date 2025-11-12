#ifndef __OPEN62541_CALLBACK_REPOSITORY_HPP
#define __OPEN62541_CALLBACK_REPOSITORY_HPP

#include "NodeId.hpp"

#include <HaSLL/Logger.hpp>
#include <Information_Model/Callable.hpp>
#include <Information_Model/Observable.hpp>
#include <Information_Model/Readable.hpp>
#include <Information_Model/Writable.hpp>
#include <boost/unordered/concurrent_node_map.hpp>
#include <open62541/server.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>

template <> class boost::hash<open62541::NodeId> {
public:
  size_t operator()(const open62541::NodeId& node_id) const {
    return UA_NodeId_hash(&node_id.base());
  }
};

inline bool operator==(const UA_NodeId& lhs, const UA_NodeId& rhs) {
  return UA_NodeId_equal(&lhs, &rhs);
}

namespace open62541 {
UA_StatusCode readNodeValue(UA_Server* server, const UA_NodeId*, void*,
    const UA_NodeId* node_id, void* node_context, UA_Boolean,
    const UA_NumericRange*, UA_DataValue* value);

UA_StatusCode writeNodeValue(UA_Server* server, const UA_NodeId*, void*,
    const UA_NodeId* node_id, void* node_context, const UA_NumericRange*,
    const UA_DataValue* value);

UA_StatusCode callNodeMethod(UA_Server* server, const UA_NodeId*, void*,
    const UA_NodeId* method_id, void* method_context, const UA_NodeId*, void*,
    size_t input_size, const UA_Variant* input, size_t output_size,
    UA_Variant* output);

using CallbackWrapper = std::variant< // clang-format off
        std::monostate,
        Information_Model::ReadablePtr, 
        Information_Model::WritablePtr,
        Information_Model::ObservablePtr, 
        Information_Model::CallablePtr
    >; // clang-format on

struct CallbackRepo {
  using CallbackMap = boost::concurrent_node_map<NodeId, CallbackWrapper>;

  CallbackRepo();
  ~CallbackRepo() = default;

  UA_StatusCode add(UA_NodeId node_id, const CallbackWrapper& wrapper);

  void remove(const UA_NodeId* node_id);

  UA_StatusCode read(const UA_NodeId* node_id, UA_DataValue* value);

  UA_StatusCode write(const UA_NodeId* node_id, const UA_DataValue* value);

  UA_StatusCode execute(const UA_NodeId* method_id, size_t input_size,
      const UA_Variant* input, size_t output_size, UA_Variant* output);

private:
  CallbackWrapper find(const UA_NodeId* node_id);

  CallbackMap callbacks_;
  HaSLL::LoggerPtr logger_;
};
using CallbackRepoPtr = std::shared_ptr<CallbackRepo>;

} // namespace open62541

#endif //__OPEN62541_CALLBACK_REPOSITORY_HPP