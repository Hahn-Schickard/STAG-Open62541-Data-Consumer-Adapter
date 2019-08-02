#ifndef __NODE_BUILDER_HPP_
#define __NODE_BUILDER_HPP_

#include "Device.hpp"
#include "NodeInformation.h"

namespace opcua_model_translator {

class NodeBuilder {
public:
  NodeBuilder(Information_Model::Device *device);
  NodeDescription *get_Node_Description();

  NodeDescription *device_node;

private:
  void add_Element_Groups();
  NodeDescription
  build_Node_Description(Information_Model::DeviceElement *device_element);
  void build_Node_Class(NodeDescription &element_node,
                        Information_Model::DeviceElement *device_element);
  void
  build_Variable_Node_Class(NodeDescription &element_node,
                            Information_Model::DeviceElement *device_element);
  void
  build_Method_Node_Class(NodeDescription &element_node,
                          Information_Model::DeviceElement *device_element);
  void build_Object_Node_Class(
      NodeDescription &element_node,
      Information_Model::DeviceElementGroup *device_element);
  NodeClassType
  identify_Element_Type(Information_Model::DeviceElement *device_element);

  Information_Model::Device *device;
};
} // namespace opcua_model_translator

#endif //__NODE_BUILDER_HPP_