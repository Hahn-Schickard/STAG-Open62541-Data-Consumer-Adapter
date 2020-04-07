#include "NodeMananger.hpp"
#include "LoggerRepository.hpp"

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace open62541;

string toString(DataType type) {
  switch (type) {
  case DataType::BOOLEAN: {
    return "BOOLEAN";
  }
  case DataType::BYTE: {
    return "BYTE";
  }
  case DataType::SHORT: {
    return "SHORT";
  }
  case DataType::INTEGER: {
    return "INTEGER";
  }
  case DataType::LONG: {
    return "LONG";
  }
  case DataType::FLOAT: {
    return "FLOAT";
  }
  case DataType::DOUBLE: {
    return "DOUBLE";
  }
  case DataType::STRING: {
    return "STRING";
  }
  case DataType::UNKNOWN:
  default: { return "UNKNOWN"; }
  }
}
NodeIDWrapper::NodeIDWrapper(const UA_NodeId *node_id)
    : node_id_((UA_NodeId *)malloc(sizeof(UA_NodeId))) {
  memcpy(node_id_, node_id, sizeof(*node_id));
}

NodeIDWrapper::~NodeIDWrapper() { delete (node_id_); }

string makeString(UA_Guid guid) {
  string result;
  string string1 = to_string(guid.data1);
  string string2 = to_string(guid.data2);
  string string3 = to_string(guid.data3);
  string string4;
  for (uint8_t i = 0; i < 8; i++) {
    string4.push_back(guid.data4[i]);
  }
  if (!string1.empty() && !string2.empty() && !string3.empty() &&
      !string4.empty()) {
    // find out if it is Bigendian or Littleendian
    result = string1 + "-" + string2 + "-" + string3 + "-" + string4;
  }
  return result;
}

string NodeIDWrapper::getString() {
  string result;
  switch (node_id_->identifierType) {
  case UA_NodeIdType::UA_NODEIDTYPE_NUMERIC: {
    result = to_string(node_id_->identifier.numeric);
    break;
  }
  case UA_NodeIdType::UA_NODEIDTYPE_BYTESTRING: {
  }
  case UA_NodeIdType::UA_NODEIDTYPE_STRING: {
    result = string((char *)node_id_->identifier.string.data,
                    node_id_->identifier.string.length);
    break;
  }
  case UA_NodeIdType::UA_NODEIDTYPE_GUID: {
    result = makeString(node_id_->identifier.guid);
    break;
  }
  default: { break; }
  }
  return result;
}

NodeManager::NodeManager()
    : logger_(LoggerRepository::getInstance().registerTypedLoger(this)) {}

NodeManager::~NodeManager() {
  LoggerRepository::getInstance().deregisterLoger(logger_->getName());
}

unordered_map<shared_ptr<NodeIDWrapper>, shared_ptr<CallbackWrapper>>::iterator
NodeManager::findIndexPosition(shared_ptr<NodeIDWrapper> node_id) {
  unordered_map<shared_ptr<NodeIDWrapper>,
                shared_ptr<CallbackWrapper>>::iterator it;
  it = node_calbacks_map_.find(node_id);
  return it;
}

shared_ptr<CallbackWrapper>
NodeManager::findCallbackWrapper(shared_ptr<NodeIDWrapper> node_id) {
  auto result = make_shared<CallbackWrapper>();
  auto it = findIndexPosition(node_id);
  if (it != node_calbacks_map_.end()) {
    logger_->log(SeverityLevel::TRACE, "Node [] callbacks found.",
                 node_id->getString());
    result = it->second;
  } else {
    logger_->log(SeverityLevel::ERROR,
                 "Node [] does not have any callbacks registered!",
                 node_id->getString());
  }
  return result;
}

UA_StatusCode NodeManager::addNode(DataType type, const UA_NodeId *nodeId,
                                   ReadCallback read_callback) {
  return addNode(type, nodeId, read_callback, nullptr);
}

UA_StatusCode NodeManager::addNode(DataType type, const UA_NodeId *nodeId,
                                   ReadCallback read_callback,
                                   WriteCallback write_callback) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  auto element_node_id = make_shared<NodeIDWrapper>(nodeId);
  if (findCallbackWrapper(element_node_id)) {
    logger_->log(SeverityLevel::TRACE, "Adding callbacks for Node [].",
                 element_node_id->getString());
    node_calbacks_map_.emplace(
        move(element_node_id),
        make_shared<CallbackWrapper>(type, read_callback, write_callback));
    status = UA_STATUSCODE_GOOD;
  } else {
    logger_->log(SeverityLevel::ERROR, "Node [] was already registered ealier!",
                 element_node_id->getString());
    status = UA_STATUSCODE_BADNODEIDEXISTS;
  }
  return status;
}

UA_StatusCode NodeManager::removeNode(const UA_NodeId *nodeId) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTSUPPORTED;
  auto element_node_id = make_shared<NodeIDWrapper>(nodeId);
  auto it = findIndexPosition(element_node_id);
  if (it != node_calbacks_map_.end()) {
    logger_->log(SeverityLevel::TRACE, "Removing callbacks for Node [].",
                 element_node_id->getString());
    node_calbacks_map_.erase(it);
    status = UA_STATUSCODE_GOOD;
  } else {
    logger_->log(SeverityLevel::ERROR,
                 "Node [] does not have any registered callbacks!",
                 element_node_id->getString());
    status = UA_STATUSCODE_BADNOTFOUND;
  }
  return status;
}

UA_StatusCode
NodeManager::readNodeValue(UA_Server *server, const UA_NodeId *sessionId,
                           void *sessionContext, const UA_NodeId *nodeId,
                           void *nodeContext, UA_Boolean includeSourceTimeStamp,
                           const UA_NumericRange *range, UA_DataValue *value) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTREADABLE;

  auto element_node_id = make_shared<NodeIDWrapper>(nodeId);
  auto it = findIndexPosition(element_node_id);
  if (it != node_calbacks_map_.end()) {
    logger_->log(SeverityLevel::TRACE, "Calling read callback for Node [].",
                 element_node_id->getString());
    auto callbacks = it->second;
    auto variant_value = callbacks->read_callback();
    try {
      match(variant_value,
            [&](bool boolean_value) {
              if (callbacks->data_type == DataType::BOOLEAN) {
                UA_Variant_setScalar(&value->value, &boolean_value,
                                     &UA_TYPES[UA_TYPES_BOOLEAN]);
              } else {
                throw runtime_error("Tried to read a Boolean data type "
                                    "when node data type is: " +
                                    toString(callbacks->data_type));
              }
            },
            [&](uint8_t byte_value) {
              if (callbacks->data_type == DataType::BYTE) {
                UA_Variant_setScalar(&value->value, &byte_value,
                                     &UA_TYPES[UA_TYPES_BYTE]);
              } else {
                throw runtime_error("Tried to read a Byte data type "
                                    "when node data type is: " +
                                    toString(callbacks->data_type));
              }
            },
            [&](int16_t short_value) {
              if (callbacks->data_type == DataType::SHORT) {
                UA_Variant_setScalar(&value->value, &short_value,
                                     &UA_TYPES[UA_TYPES_INT16]);
              } else {
                throw runtime_error("Tried to read a Short data type "
                                    "when node data type is: " +
                                    toString(callbacks->data_type));
              }
            },
            [&](int32_t integer_value) {
              if (callbacks->data_type == DataType::INTEGER) {
                UA_Variant_setScalar(&value->value, &integer_value,
                                     &UA_TYPES[UA_TYPES_INT32]);
              } else {
                throw runtime_error("Tried to read a Integer data type "
                                    "when node data type is:" +
                                    toString(callbacks->data_type));
              }
            },
            [&](int64_t long_value) {
              if (callbacks->data_type == DataType::LONG) {
                UA_Variant_setScalar(&value->value, &long_value,
                                     &UA_TYPES[UA_TYPES_INT64]);
              } else {
                throw runtime_error("Tried to read a Long data type "
                                    "when node data type is: " +
                                    toString(callbacks->data_type));
              }
            },
            [&](float float_value) {
              if (callbacks->data_type == DataType::FLOAT) {
                UA_Variant_setScalar(&value->value, &float_value,
                                     &UA_TYPES[UA_TYPES_FLOAT]);
              } else {
                throw runtime_error("Tried to read a Float data type "
                                    "when node data type is: " +
                                    toString(callbacks->data_type));
              }
            },
            [&](double double_value) {
              if (callbacks->data_type == DataType::DOUBLE) {
                UA_Variant_setScalar(&value->value, &double_value,
                                     &UA_TYPES[UA_TYPES_DOUBLE]);
              } else {
                throw runtime_error("Tried to read a Double data type "
                                    "when node data type is:" +
                                    toString(callbacks->data_type));
              }
            },
            [&](string string_value) {
              if (callbacks->data_type == DataType::STRING) {
                UA_String open62541_string;
                open62541_string.length = strlen(string_value.c_str());
                open62541_string.data =
                    (UA_Byte *)malloc(open62541_string.length);
                memcpy(open62541_string.data, string_value.c_str(),
                       open62541_string.length);
                UA_Variant_setScalar(&value->value, &open62541_string,
                                     &UA_TYPES[UA_TYPES_STRING]);
              } else {
                throw runtime_error("Tried to read a String data type "
                                    "when node data type is: " +
                                    toString(callbacks->data_type));
              }
            });
      value->hasValue = true;
      status = UA_STATUSCODE_GOOD;
    } catch (const exception &exp) {
      logger_->log(SeverityLevel::ERROR,
                   "Node [] does not contain the requested data type: []",
                   element_node_id->getString(), exp.what());
      status = UA_STATUSCODE_BADTYPEMISMATCH;
    }
  } else {
    logger_->log(SeverityLevel::ERROR,
                 "Node [] does not have any registered read callbacks!",
                 element_node_id->getString());
  }
  return status;
}

UA_StatusCode
NodeManager::writeNodeValue(UA_Server *server, const UA_NodeId *sessionId,
                            void *sessionContext, const UA_NodeId *nodeId,
                            void *nodeContext, const UA_NumericRange *range,
                            const UA_DataValue *value) {
  UA_StatusCode status = UA_STATUSCODE_BADNOTWRITABLE;
  auto element_node_id = make_shared<NodeIDWrapper>(nodeId);
  auto it = findIndexPosition(element_node_id);
  if (it != node_calbacks_map_.end()) {
    auto callbacks = it->second;
    if (it->second->write_callback.has_value()) {
      auto write_CB = callbacks->write_callback.value();
      switch (value->value.type->typeKind) {
      case UA_DataTypeKind::UA_DATATYPEKIND_BOOLEAN: {
        bool boolean_value = *((bool *)(value->value.data));
        write_CB(DataVariant(boolean_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_BYTE: {
        uint8_t byte_value = *((uint8_t *)(value->value.data));
        write_CB(DataVariant(byte_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_SBYTE:
      case UA_DataTypeKind::UA_DATATYPEKIND_UINT16:
      case UA_DataTypeKind::UA_DATATYPEKIND_INT16: {
        int16_t short_value = *((int16_t *)(value->value.data));
        write_CB(DataVariant(short_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_UINT32:
      case UA_DataTypeKind::UA_DATATYPEKIND_INT32: {
        int32_t integer_value = *((int32_t *)(value->value.data));
        write_CB(DataVariant(integer_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_DATETIME:
      case UA_DataTypeKind::UA_DATATYPEKIND_UINT64:
      case UA_DataTypeKind::UA_DATATYPEKIND_INT64: {
        int64_t long_value = *((int64_t *)(value->value.data));
        write_CB(DataVariant(long_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_FLOAT: {
        float float_value = *((float *)(value->value.data));
        write_CB(DataVariant(float_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_DOUBLE: {
        double double_value = *((double *)(value->value.data));
        write_CB(DataVariant(double_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_BYTESTRING:
      case UA_DataTypeKind::UA_DATATYPEKIND_STATUSCODE:
      case UA_DataTypeKind::UA_DATATYPEKIND_STRING: {
        UA_String *ua_string = (UA_String *)(value->value.data);
        auto string_value = string((char *)ua_string->data, ua_string->length);
        write_CB(DataVariant(string_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      case UA_DataTypeKind::UA_DATATYPEKIND_GUID: {
        UA_Guid ua_guid;
        ua_guid.data1 = ((UA_Guid *)(value->value.data))->data1;
        ua_guid.data2 = ((UA_Guid *)(value->value.data))->data2;
        ua_guid.data3 = ((UA_Guid *)(value->value.data))->data3;
        for (uint8_t i = 0; i < 8; i++) {
          ua_guid.data4[i] = ((UA_Guid *)(value->value.data))->data4[i];
        }
        auto string_value = makeString(ua_guid);
        write_CB(DataVariant(string_value));
        status = UA_STATUSCODE_GOOD;
        break;
      }
      default: {
        logger_->log(
            SeverityLevel::ERROR,
            "Node [] does not take [] as its argument for write callback!",
            element_node_id->getString(), string(value->value.type->typeName));
        status = UA_STATUSCODE_BADINVALIDARGUMENT;
        break;
      }
      }
    } else {
      logger_->log(SeverityLevel::ERROR,
                   "Node [] does not have a registered write callback!",
                   element_node_id->getString());
    }
  } else {
    logger_->log(SeverityLevel::ERROR,
                 "Node [] does not have any registered callbacks!",
                 element_node_id->getString());
    status = UA_STATUSCODE_BADNOTFOUND;
  }
  return status;
}