#ifndef __NODE_BUILDER_HPP_
#define __NODE_BUILDER_HPP_

#include "NodeInformation.h"
#include "device.hpp"

namespace opcua_model_translator
{

class NodeBuilder
{
public:
  NodeBuilder(Device *device);
  NodeDescription *get_Node_Description();

  NodeDescription *device_node;

private:
  void add_Element_Groups();
  NodeDescription build_Node_Description(DeviceElement *device_element);
  void build_Node_Class(NodeDescription &element_node, DeviceElement *device_element);
  void build_Variable_Node_Class(NodeDescription &element_node, DeviceElement *device_element);
  void build_Method_Node_Class(NodeDescription &element_node, DeviceElement *device_element);
  void build_Object_Node_Class(NodeDescription &element_node, DeviceElementGroup *device_element);
  NodeClassType identify_Element_Type(DeviceElement *device_element);
  Device *device_base;
};
} // namespace opcua_model_translator

#endif //__NODE_BUILDER_HPP_