#include "Event_Model/EventSource.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "Information_Model/mocks/Metric_MOCK.hpp"
#include "LoggerRepository.hpp"
#include "OpcuaAdapter.hpp"

#include <gmock/gmock.h>
#include <iostream>
#include <signal.h>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace DCAI;

using ::testing::NiceMock;

OpcuaAdapter *adapter;

void stopServer() {
  adapter->stop();
  delete adapter;
}

static void stopHandler(int /*sig*/) {
  cout << "Received stop signal!" << endl;
  stopServer();
  exit(0);
}

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
public:
  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

void print(DevicePtr device);
void print(DeviceElementPtr element, size_t offset);
void print(WritableMetricPtr element, size_t offset);
void print(MetricPtr element, size_t offset);
void print(DeviceElementGroupPtr elements, size_t offset);

int main(int argc, char *argv[]) {
  /*
    CL format:
    - First argument (argv[1]): Path of server config file (required)
    - Second argument: Server lifetime in s (infinite lifetime if omitted)
  */

  if (argc < 2) {
    cerr << "Required CL argument: server config filepath" << endl;
    return -1;
  }

  try {
    auto config = HaSLL::Configuration(
        "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
        HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
    HaSLL::LoggerRepository::initialise(config);
    auto logger = HaSLL::LoggerRepository::getInstance().registerLoger("Main");
    logger->log(SeverityLevel::TRACE, "Logging completed initialization!");

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    logger->log(
        SeverityLevel::TRACE,
        "Termination and Interuption signals assigned to stop handler!");

    auto event_source = make_shared<EventSourceFake>();
    logger->log(SeverityLevel::TRACE, "Fake event source initialized!");

    adapter = new OpcuaAdapter(event_source, string(argv[1]));
    logger->log(SeverityLevel::TRACE, "OPC UA Addapter initialized!");

    adapter->start();

    {
      auto mock_builder =
          make_shared<Information_Model::testing::DeviceMockBuilder>();

      mock_builder->buildDeviceBase("9876", "Mocky", "Mocked test device");
      logger->log(SeverityLevel::TRACE, "Started building Mock Device.");

      auto subgroup_1_ref_id =
          mock_builder->addDeviceElementGroup("Group 1", "First group");
      logger->log(SeverityLevel::TRACE,
                  "Adding a Subgroup element with id {} to Mock Device.",
                  subgroup_1_ref_id);

      auto boolean_ref_id = mock_builder->addReadableMetric(
          subgroup_1_ref_id, "Boolean", "Mocked readable metric",
          Information_Model::DataType::BOOLEAN,
          []() -> DataVariant { return true; });
      logger->log(
          SeverityLevel::TRACE,
          "Adding a Boolean readable element with id {} to Mock Device.",
          boolean_ref_id);

      auto integer_ref_id = mock_builder->addReadableMetric(
          "Integer", "Mocked readable metric",
          Information_Model::DataType::INTEGER,
          []() -> DataVariant { return (int64_t)48; });
      logger->log(
          SeverityLevel::TRACE,
          "Adding an Integer readable element with id {} to Mock Device.",
          integer_ref_id);

      auto string_ref_id = mock_builder->addReadableMetric(
          "String", "Mocked readable metric",
          Information_Model::DataType::STRING,
          []() -> DataVariant { return string("Hello World!"); });
      logger->log(SeverityLevel::TRACE,
                  "Adding a String readable element with id {} to Mock Device.",
                  string_ref_id);

      auto device = mock_builder->getResult();

      mock_builder.reset();

      print(device);

      event_source->sendEvent(make_shared<ModelRegistryEvent>(device));
    }

    if (argc > 2) {
      uint server_lifetime = atoi(argv[2]);
      cout << "Open62541 server will automatically shut down in "
           << server_lifetime << " seconds." << endl;
      sleep(server_lifetime);
      stopServer();
    } else {
      while (true)
        ;
    }
  } catch (exception &ex) {
    cerr << ex.what() << endl;
  }
}

void print(DeviceElementGroupPtr elements, size_t offset) {
  cout << string(offset, ' ') << "Group contains elements:" << endl;
  cout << string(offset, ' ') << "[" << endl;
  for (auto element : elements->getSubelements()) {
    print(element, offset + 3);
  }
  cout << string(offset, ' ') << "]" << endl;
}

void print(MetricPtr element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->getDataType())
       << " value: " << toString(element->getMetricValue()) << endl;
}

void print(WritableMetricPtr element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->getDataType())
       << " value: " << toString(element->getMetricValue()) << endl;
  cout << string(offset, ' ') << "Writes " << toString(element->getDataType())
       << " value type" << endl;
}

void print(DeviceElementPtr element, size_t offset) {
  cout << string(offset - 1, ' ') << "{" << endl;
  cout << string(offset, ' ') << "Element name: " << element->getElementName()
       << endl;
  cout << string(offset, ' ') << "Element id: " << element->getElementId()
       << endl;
  cout << string(offset, ' ')
       << "Described as: " << element->getElementDescription() << endl;
  cout << string(offset, ' ')
       << "Element type: " << toString(element->getElementType()) << endl;

  switch (element->getElementType()) {
  case ElementType::GROUP: {
    print(static_pointer_cast<DeviceElementGroup>(element), offset);
    break;
  }
  case ElementType::READABLE: {
    print(static_pointer_cast<Metric>(element), offset);
    break;
  }
  case ElementType::WRITABLE: {
    print(static_pointer_cast<WritableMetric>(element), offset);
    break;
  }
  case ElementType::FUNCTION: {
    cerr << string(offset, ' ') << "Function element types are not implemented!"
         << endl;
    break;
  }
  case ElementType::OBSERVABLE: {
    cerr << string(offset, ' ')
         << "Observable elements types are not implemented!" << endl;
    break;
  }
  case ElementType::UNDEFINED:
  default: {
    cerr << string(offset, ' ') << "Is not a valid element type!" << endl;
    break;
  }
  }
  cout << string(offset - 1, ' ') << "}" << endl;
}

void print(DevicePtr device) {
  cout << "Device name: " << device->getElementName() << endl;
  cout << "Device id: " << device->getElementId() << endl;
  cout << "Described as: " << device->getElementDescription() << endl;
  print(device->getDeviceElementGroup(), 0);
}
