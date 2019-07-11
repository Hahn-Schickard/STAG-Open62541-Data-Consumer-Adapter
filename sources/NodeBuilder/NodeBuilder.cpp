#include "NodeBuilder.hpp"
#include "CStringFormater.hpp"

using namespace opcua_model_translator;

NodeBuilder::NodeBuilder(Device *device)
{
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

void NodeBuilder::add_Element_Groups()
{
  std::vector<DeviceElementGroup *> device_element_groups =
      device_base->getDeviceElementGroups();

  device_node->object_node_descriptor.children_count =
      device_element_groups.size();

  for (std::size_t i = 0; i < device_element_groups.size(); i++)
  {
    device_node->object_node_descriptor.children[i] = build_Node_Description(device_element_groups[i]);
  }
}

NodeDescription NodeBuilder::build_Node_Description(DeviceElement *device_element)
{
  NodeDescription element_node;

  copyCharArray(device_element->getElementName().c_str(), element_node.base_node_descriptor.display_name);
  copyCharArray(element_node.base_node_descriptor.display_name, element_node.base_node_descriptor.browse_name);
  copyCharArray("EN_US", element_node.base_node_descriptor.locale);
  element_node.base_node_descriptor.node_class = identify_Element_Type(device_element);
  copyCharArray(device_element->getElementDescription().c_str(), element_node.base_node_descriptor.description);
  copyCharArray(device_element->getReferenceId().c_str(), element_node.base_node_descriptor.unique_id);
  build_Node_Class(element_node, device_element);

  return element_node;
}

void NodeBuilder::build_Node_Class(NodeDescription &element_node, DeviceElement *device_element)
{
  switch (element_node.base_node_descriptor.node_class)
  {
  case NodeClassType::VARIABLE_NODE:
  {
    build_Variable_Node_Class(element_node, device_element);
    break;
  }
  case NodeClassType::METHOD_NODE:
  {
    build_Method_Node_Class(element_node, device_element);
    break;
  }
  case NodeClassType::OBJECT_NODE:
  {
    build_Object_Node_Class(element_node, (DeviceElementGroup *)device_element);
    break;
  }
  case NodeClassType::UNRECOGNIZED_NODE:
  default:
  {
    // TODO: Handle default behaviour
    break;
  }
  }
}

void NodeBuilder::build_Variable_Node_Class(NodeDescription &element_node, DeviceElement *device_element)
{
  // TODO: wait until valueDataType has been made accessable.
  //element_node.variable_node_descriptor.data.type = device_element->getMetricValueType();
}

void NodeBuilder::build_Method_Node_Class(NodeDescription &element_node, DeviceElement *device_element)
{
  // TODO: handle method building
}

void NodeBuilder::build_Object_Node_Class(NodeDescription &element_node, DeviceElementGroup *device_element)
{
  std::vector<DeviceElement *> device_subelements =
      device_element->getSubElements();
  std::vector<DeviceElementGroup *> debice_subgroups = device_element->getSubElementGroups();
  element_node.object_node_descriptor.children_count = debice_subgroups.size() + debice_subgroups.size();
  for (std::size_t i = 0; i < device_subelements.size(); i++)
  {
    element_node.object_node_descriptor.children[i] = build_Node_Description(device_subelements[i]);
  }
  for (std::size_t i = device_subelements.size(); i < element_node.object_node_descriptor.children_count; i++)
  {
    element_node.object_node_descriptor.children[i] = build_Node_Description(debice_subgroups[i]);
  }
}

NodeClassType
NodeBuilder::identify_Element_Type(DeviceElement *device_element)
{
  NodeClassType node_type;
  ElementType element_type = device_element->getElementType();
  switch (element_type)
  {
  case Group:
  {
    node_type = NodeClassType::OBJECT_NODE;
    break;
  }
  case Readonly:
  {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Observable:
  {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Writable:
  {
    node_type = NodeClassType::VARIABLE_NODE;
    break;
  }
  case Function:
  {
    node_type = NodeClassType::METHOD_NODE;
    break;
  }
  case Undefined:
  default:
  {
    node_type = NodeClassType::UNRECOGNIZED_NODE;
    break;
  }
  }
  return node_type;
}

NodeDescription *NodeBuilder::get_Node_Description() { return device_node; }