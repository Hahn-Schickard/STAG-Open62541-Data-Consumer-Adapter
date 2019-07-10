#ifndef __NODE_BUILDER_HPP_
#define __NODE_BUILDER_HPP_

#include "NodeInformation.h"
#include "device.hpp"

namespace opcua_model_translator {

class NodeBuilder {
public:
  NodeBuilder(Device *device);
  NodeDescription *get_Node_Description();

  NodeDescription *device_node;

private:
  void add_Element_Groups();
  template <typename T>
  NodeDescription build_Node_Description(T *device_element);
  NodeClassType identify_Element_Type(DeviceElementGroup *device_element);
  Device *device_base;
};
}

#endif //__NODE_BUILDER_HPP_