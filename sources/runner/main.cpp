#include "DeviceBuilder.hpp"
#include "Notifier.hpp"
#include "OpcuaAdapter.hpp"
#include <iostream>
#include <memory>
#include <unistd.h>

using namespace std;

#define SERVER_LIFETIME 60

int main() {
  OpcuaAdapter *server = new OpcuaAdapter();

  Model_Factory::DeviceBuilder *abstraction_builder =
      new Model_Factory::DeviceBuilder("TestDevice", "1234",
                                       "A hardcoded device for testing only");
  abstraction_builder->addDeviceElementGroup(
      "RootGroup", "A root group containign all of the elements within device");
  abstraction_builder->addDeviceElement(
      "SimpleElement", "This is a simple element",
      Information_Model::ElementType::Readonly);
  std::unique_ptr<Information_Model::Device> device =
      abstraction_builder->getDevice();

  server->startOpen62541();

  server->handleEvent(device.get());

  sleep(SERVER_LIFETIME);

  exit(0);
}