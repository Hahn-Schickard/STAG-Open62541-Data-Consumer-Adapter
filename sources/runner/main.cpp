#include "Event_Model/EventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "Information_Model/mocks/Metric_MOCK.hpp"

#include "OpcuaAdapter.hpp"

#include <csignal>
#include <exception>
#include <gmock/gmock.h>
#include <iostream>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace DCAI;

using ::testing::NiceMock;

OpcuaAdapter* adapter;

void stopServer() {
  adapter->stop();
  delete adapter;
}

static void stopHandler(int /*sig*/) {
  cout << "Received stop signal!" << endl;
  stopServer();
  exit(EXIT_SUCCESS);
}

void printException(const exception& e, int level = 0) {
  cerr << string(level, ' ') << "Exception: " << e.what() << endl;
  try {
    rethrow_if_nested(e);
  } catch (const exception& nested_exception) {
    printException(nested_exception, level + 1);
  } catch (...) {
  }
}

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
  void handleException(exception_ptr eptr) { // NOLINT
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  void sendEvent(ModelRegistryEventPtr event) { notify(event); } // NOLINT
};

void print(NonemptyDevicePtr device);
void print(NonemptyDeviceElementPtr element, size_t offset);
void print(NonemptyWritableMetricPtr element, size_t offset);
void print(NonemptyMetricPtr element, size_t offset);
void print(NonemptyDeviceElementGroupPtr elements, size_t offset);

int main(int argc, char* argv[]) {
  try {
    auto repo = make_shared<SPD_LoggerRepository>("config/loggerConfig.json");
    LoggerManager::initialise(repo);
    auto logger = HaSLL::LoggerManager::registerLogger("Main");
    logger->log(SeverityLevel::TRACE, "Logging completed initialization!");

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    logger->log(SeverityLevel::TRACE,
        "Termination and Interruption signals assigned to stop handler!");

    auto event_source = make_shared<EventSourceFake>();
    logger->log(SeverityLevel::TRACE, "Fake event source initialized!");

    adapter = new OpcuaAdapter(event_source, "config/defaultConfig.json");
    logger->log(SeverityLevel::TRACE, "OPC UA Adapter initialized!");

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

      auto boolean_ref_id =
          mock_builder->addReadableMetric(subgroup_1_ref_id, "Boolean",
              "Mocked readable metric", Information_Model::DataType::BOOLEAN,
              []() -> DataVariant { return true; });
      logger->log(SeverityLevel::TRACE,
          "Adding a Boolean readable element with id {} to Mock Device.",
          boolean_ref_id);

      auto integer_ref_id = mock_builder->addReadableMetric("Integer",
          "Mocked readable metric", Information_Model::DataType::INTEGER,
          []() -> DataVariant { return (int64_t)48; }); // NOLINT
      logger->log(SeverityLevel::TRACE,
          "Adding an Integer readable element with id {} to Mock Device.",
          integer_ref_id);

      auto string_ref_id = mock_builder->addReadableMetric("String",
          "Mocked readable metric", Information_Model::DataType::STRING,
          []() -> DataVariant { return string("Hello World!"); });
      logger->log(SeverityLevel::TRACE,
          "Adding a String readable element with id {} to Mock Device.",
          string_ref_id);

      auto device = mock_builder->getResult();

      mock_builder.reset();

      if (device) {
        print(NonemptyDevicePtr(device));
      }

      event_source->sendEvent(
          std::make_shared<ModelRegistryEvent>(NonemptyDevicePtr(device)));
    }

    if (argc > 1) {
      uint server_lifetime = atoi(argv[1]);
      cout << "Open62541 server will automatically shut down in "
           << server_lifetime << " seconds." << endl;
      sleep(server_lifetime);
      stopServer();
    } else {
      while (true) {
        ;
      }
    }
  } catch (const exception& ex) {
    printException(ex);
    exit(EXIT_FAILURE);
  } catch (...) {
    cerr << "Unknown error occurred during program execution" << endl;
    exit(EXIT_FAILURE);
  }
}

void print(NonemptyDeviceElementGroupPtr elements, size_t offset) {
  cout << string(offset, ' ') << "Group contains elements:" << endl;
  cout << string(offset, ' ') << "[" << endl;
  for (auto element : elements->getSubelements()) {
    print(element, offset + 3);
  }
  cout << string(offset, ' ') << "]" << endl;
}

void print(NonemptyMetricPtr element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->getDataType())
       << " value: " << toString(element->getMetricValue()) << endl;
}

void print(NonemptyWritableMetricPtr element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->getDataType())
       << " value: " << toString(element->getMetricValue()) << endl;
  cout << string(offset, ' ') << "Writes " << toString(element->getDataType())
       << " value type" << endl;
}

void print(NonemptyDeviceElementPtr element, size_t offset) {
  cout << string(offset - 1, ' ') << "{" << endl;
  cout << string(offset, ' ') << "Element name: " << element->getElementName()
       << endl;
  cout << string(offset, ' ') << "Element id: " << element->getElementId()
       << endl;
  cout << string(offset, ' ')
       << "Described as: " << element->getElementDescription() << endl;

  match(
      element->specific_interface,
      [offset](NonemptyDeviceElementGroupPtr interface) {
        print(interface, offset);
      },
      [offset](NonemptyMetricPtr interface) { print(interface, offset); },
      [offset](
          NonemptyWritableMetricPtr interface) { print(interface, offset); });

  cout << string(offset - 1, ' ') << "}" << endl;
}

void print(NonemptyDevicePtr device) {
  cout << "Device name: " << device->getElementName() << endl;
  cout << "Device id: " << device->getElementId() << endl;
  cout << "Described as: " << device->getElementDescription() << endl;
  print(device->getDeviceElementGroup(), 0);
}
