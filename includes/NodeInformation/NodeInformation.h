#ifndef NODE_INFORMATION_H
#define NODE_INFORMATION_H

#include <stdint.h>

struct BaseNodeInformation;
struct VariableNodeInformation;
struct MethodNodeInformation;
struct ObjectNodeInformation;
struct NodeInformation;

typedef BaseNodeInformation BaseNodeDescription;
typedef VariableNodeInformation VariableNodeDescription;
typedef MethodNodeInformation MethodNodeDescription;
typedef ObjectNodeInformation ObjectNodeDescription;
typedef NodeInformation NodeDescription;

enum DataType {
  UnsignedShort,
  UnsignedInteger,
  UnsignedLong,
  SignedShort,
  SignedInteger,
  SignedLong,
  Double,
  Boolean,
  String,
  Unknown
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

struct BaseNodeInformation {
  NodeClassType nodeClass;
  bool writableFlag;
  char uniqueId[128];
  char locale[6];
  char displayName[256];
  char browseName[256];
  char description[256];
};

struct VariableNodeInformation {
  DataType dataType;
  DataValue dataValue;
};

struct MethodNodeInformation {
  // TODO: Handle methods
};

struct ObjectNodeInformation {
  uint16_t childrenCount;
  NodeDescription *children;
};

struct NodeInformation {
  BaseNodeDescription baseNodeDescriptor;
  union {
    VariableNodeDescription variableNodeDescriptor;
    MethodNodeDescription methodNodeDescriptor;
    ObjectNodeDescription objectNodeDescriptor;
  };
};

#endif // NODE_INFORMATION_H