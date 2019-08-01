#include "ConsumerNotifier.hpp"
#include "NodeBuilder.hpp"

using namespace OPCUA_Notifier;
using namespace opcua_model_translator;

void ConsumerNotifier::handleEvent(Information_Model::Device *device) {
  NodeBuilder *node_builder = new NodeBuilder(device);
  NodeDescription *node_descriptor = node_builder->get_Node_Description();
  update_DataConsumer(node_descriptor);
  delete node_builder;
}