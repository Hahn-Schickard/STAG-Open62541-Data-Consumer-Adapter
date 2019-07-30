#ifndef NODE_INFORMATION_H
#define NODE_INFORMATION_H

#include <stdint.h>
#include <stdbool.h>

struct NodeInformation;

typedef struct NodeInformation NodeDescription;

typedef enum DataTypeEnum
{
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
} DataType;

typedef enum NodeClassTypeEnum
{
  VARIABLE_NODE,
  METHOD_NODE,
  OBJECT_NODE,
  UNRECOGNIZED_NODE
} NodeClassType;

typedef union DataValueUnion {
  uint8_t unsigned_short_value;
  int8_t signed_short_value;
  uint32_t unsigned_integer_value;
  int32_t signed_integer_value;
  uint64_t unsigned_long_value;
  int64_t signed_long_value;
  double double_value;
  bool boolean_value;
  char *string_value;
} DataValue;

typedef struct DataValueWrapperStruct
{
  DataType type;
  DataValue value;
} DataValueWrapper;

typedef struct BaseNodeInformation
{
  NodeClassType node_class;
  bool writable_flag;
  char locale[8];
  char unique_id[128];
  char display_name[256];
  char browse_name[256];
  char description[256];
} BaseNodeDescription;

typedef struct VariableNodeInformation
{
  DataValueWrapper data;
} VariableNodeDescription;

typedef struct MethodNodeInformation
{
  // TODO: Handle methods
} MethodNodeDescription;

typedef struct ObjectNodeInformation
{
  uint16_t children_count;
  NodeDescription *children;
} ObjectNodeDescription;

struct NodeInformation
{
  BaseNodeDescription base_node_descriptor;
  union {
    VariableNodeDescription variable_node_descriptor;
    MethodNodeDescription method_node_descriptor;
    ObjectNodeDescription object_node_descriptor;
  };
};

#endif // NODE_INFORMATION_H
