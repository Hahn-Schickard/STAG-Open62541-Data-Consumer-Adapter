#ifndef NODE_INFORMATION_H
#define NODE_INFORMATION_H

#include <stdint.h>

struct BaseNodeInformation;
struct VariableNodeInformation;
struct MethodNodeInformation;
struct ObjectNodeInformation;
struct NodeInformation;
struct DataValueWrapper;

typedef BaseNodeInformation BaseNodeDescription;
typedef VariableNodeInformation VariableNodeDescription;
typedef MethodNodeInformation MethodNodeDescription;
typedef ObjectNodeInformation ObjectNodeDescription;
typedef NodeInformation NodeDescription;
typedef DataValueWrapper DataWrapper;

enum DataType {
  UNSIGNED_SHORT,
  UNSIGNED_INTEGER,
  UNSIGNED_LONG,
  SIGNED_SHORT,
  SIGNED_INTEGER,
  SIGNED_LONG,
  DOUBLE,
  BOOLEAN,
  STRING,
  UNKNOWN
};

enum NodeClassType {
  VARIABLE_NODE,
  METHOD_NODE,
  OBJECT_NODE,
  UNRECOGNIZED_NODE
};

union DataValue {
  uint8_t unsigned_short_value;
  int8_t signed_short_value;
  uint32_t unsigned_integer_value;
  int32_t signed_integer_value;
  uint64_t unsigned_long_value;
  int64_t signed_long_value;
  double double_value;
  bool boolean_value;
  char *string_value;
};

struct DataValueWrapper {
  DataType type;
  DataValue value;
};

struct BaseNodeInformation {
  NodeClassType node_class;
  bool writable_flag;
  char locale[8];
  char unique_id[128];
  char display_name[256];
  char browse_name[256];
  char description[256];
};

struct VariableNodeInformation {
  DataValueWrapper data;
};

struct MethodNodeInformation {
  // TODO: Handle methods
};

struct ObjectNodeInformation {
  uint16_t children_count;
  NodeDescription *children;
};

struct NodeInformation {
  BaseNodeDescription base_node_descriptor;
  union {
    VariableNodeDescription variable_node_descriptor;
    MethodNodeDescription method_node_descriptor;
    ObjectNodeDescription object_node_descriptor;
  };
};

#endif // NODE_INFORMATION_H