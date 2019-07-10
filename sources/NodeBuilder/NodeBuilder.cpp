#include "NodeBuilder.hpp"
#include "CStringFormater.hpp"

using namespace opcua_model_translator;

NodeBuilder::NodeBuilder(Device *device) {
  device_base = device;

  copyCharArray(device_base->getElementName().c_str(),
                device_node->base_node_descriptor.display_name);
  copyCharArray(device_node->base_node_descriptor.display_name,
                device_node->base_node_descriptor.browse_name);
  copyCharArray("EN_US", device_node->base_node_descriptor.locale);
  device_node->base_node_descriptor.node_class = NodeClassType::OBJECT_NODE;
  copyCharArray(device_base->getElementDescription().c_str(),
                device_node->base_node_descriptor.description);
  copyCharArray(device_base->getReferenceId().c_str(),
                device_node->base_node_descriptor.unique_id);
  add_Element_Groups();
}

void NodeBuilder::add_Element_Groups() {
  std::vector<DeviceElementGroup *> device_element_groups =
      device_base->getDeviceElementGroups();

  device_node->object_node_descriptor.children_count =
      device_element_groups.size();

  for (std::size_t i = 0; i < device_element_groups.size(); i++) {
    DeviceElementGroup *device_element = device_element_groups[i];
    copyCharArray(device_element->getElementName().c_str(),
                  device_node->object_node_descriptor.children[i]
                      .base_node_descriptor.display_name);
    copyCharArray(device_node->object_node_descriptor.children[i]
                      .base_node_descriptor.display_name,
                  device_node->object_node_descriptor.children[i]
                      .base_node_descriptor.browse_name);
    copyCharArray("EN_US", device_node->object_node_descriptor.children[i]
                               .base_node_descriptor.locale);
    device_node->object_node_descriptor.children[i]
        .base_node_descriptor.node_class =
        identify_Element_Type(device_element);
    copyCharArray(device_element->getElementDescription().c_str(),
                  device_node->object_node_descriptor.children[i]
                      .base_node_descriptor.description);
    copyCharArray(device_element->getReferenceId().c_str(),
                  device_node->object_node_descriptor.children[i]
                      .base_node_descriptor.unique_id);
  }
}

template <typename T>
NodeDescription NodeBuilder::build_Node_Description(T *device_element) {
  NodeDescription device_node;
  // TODO: implement stub
  return device_node;
}

NodeClassType
NodeBuilder::identify_Element_Type(DeviceElementGroup *device_element) {
  NodeClassType node_type;
  ElementType element_type = device_element->getElementType();
  switch (element_type) {
  case Group: {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Readonly: {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Observable: {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Writable: {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Function: {
    node_type = NodeClassType::METHOD_NODE;
    break;
  }
  case Undefined:
  default: {
    node_type = NodeClassType::UNRECOGNIZED_NODE;
    break;
  }
  }
  return node_type;
}

NodeDescription *NodeBuilder::get_Node_Description() { return device_node; }