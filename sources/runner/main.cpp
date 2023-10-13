#include "Event_Model/EventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "Information_Model/mocks/Metric_MOCK.hpp"

#include "OpcuaAdapter.hpp"

#include <chrono>
#include <csignal>
#include <exception>
#include <gmock/gmock.h>
#include <iostream>
#include <thread>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace Data_Consumer_Adapter;

using ::testing::NiceMock;

unique_ptr<OpcuaAdapter> adapter;

void stopServer() {
  adapter->stop();
  adapter.reset();
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

class EventSourceFake : public Event_Model::EventSource<ModelRepositoryEvent> {
  void handleException(exception_ptr eptr) { // NOLINT
    if (eptr) {
      rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  void sendEvent(const ModelRepositoryEventPtr& event) { notify(event); }
};

void print(const NonemptyDevicePtr& device);
void print(const NonemptyDeviceElementPtr& element, size_t offset);
void print(const NonemptyWritableMetricPtr& element, size_t offset);
void print(const NonemptyMetricPtr& element, size_t offset);
void print(const NonemptyDeviceElementGroupPtr& elements, size_t offset);

void registerDevices(const shared_ptr<EventSourceFake>& event_source);
void deregisterDevices(const shared_ptr<EventSourceFake>& event_source);

int main(int argc, char* argv[]) {
  try {
    auto repo = make_shared<SPD_LoggerRepository>("config/loggerConfig.json");
    LoggerManager::initialise(repo);
    auto logger = HaSLL::LoggerManager::registerLogger("Main");
    logger->log(SeverityLevel::TRACE, "Logging completed initialization!");
    logger->log(SeverityLevel::INFO,
        "Current Sever time, in number of 100 nanosecond intervals since "
        "January 1, 1601 (UTC): {}",
        UA_DateTime_now());

    signal(SIGINT, stopHandler); // NOLINT(cert-err33-c)
    signal(SIGTERM, stopHandler); // NOLINT(cert-err33-c)

    logger->log(SeverityLevel::TRACE,
        "Termination and Interruption signals assigned to stop handler!");

    auto event_source = make_shared<EventSourceFake>();
    logger->log(SeverityLevel::TRACE, "Fake event source initialized!");

    adapter =
        make_unique<OpcuaAdapter>(event_source, "config/defaultConfig.json");
    logger->log(SeverityLevel::TRACE, "OPC UA Adapter initialized!");

    adapter->start();
    registerDevices(event_source);
    this_thread::sleep_for(10s);
    logger->log(SeverityLevel::TRACE, "Sending device deregistered event");
    deregisterDevices(event_source);
    this_thread::sleep_for(5s);
    registerDevices(event_source);

    if (argc > 1) {
      auto server_lifetime = stoi(argv[1]);
      cout << "Open62541 server will automatically shut down in "
           << server_lifetime << " seconds." << endl;
      this_thread::sleep_for(chrono::seconds(server_lifetime));
      stopServer();
    } else {
      while (true) {
        this_thread::sleep_for(1s);
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

static constexpr size_t BASE_OFFSET = 160;
static constexpr size_t ELEMENT_OFFSET = 3;

void print(const NonemptyDeviceElementGroupPtr& elements, size_t offset) {
  cout << string(offset, ' ') << "Group contains elements:" << endl;
  for (const auto& element : elements->getSubelements()) {
    print(element, offset + ELEMENT_OFFSET);
  }
}

void print(const NonemptyMetricPtr& element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->getDataType())
       << " value: " << toString(element->getMetricValue()) << endl;
  cout << endl;
}

void print(const NonemptyWritableMetricPtr& element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->getDataType())
       << " value: " << toString(element->getMetricValue()) << endl;
  cout << string(offset, ' ') << "Writes " << toString(element->getDataType())
       << " value type" << endl;
  cout << endl;
}

void print(const NonemptyFunctionPtr& element, size_t offset) {
  cout << string(offset, ' ') << "Executes " << toString(element->result_type)
       << " (" << toString(element->parameters) << ")" << endl;
}

void print(const NonemptyDeviceElementPtr& element, size_t offset) {
  cout << string(offset, ' ') << "Element name: " << element->getElementName()
       << endl;
  cout << string(offset, ' ') << "Element id: " << element->getElementId()
       << endl;
  cout << string(offset, ' ')
       << "Described as: " << element->getElementDescription() << endl;

  match(
      element->functionality,
      [offset](const NonemptyDeviceElementGroupPtr& interface) {
        print(interface, offset);
      },
      [offset](
          const NonemptyMetricPtr& interface) { print(interface, offset); },
      [offset](const NonemptyWritableMetricPtr& interface) {
        print(interface, offset);
      },
      [offset](
          const NonemptyFunctionPtr& interface) { print(interface, offset); });
}

void print(const NonemptyDevicePtr& device) {
  cout << "Device name: " << device->getElementName() << endl;
  cout << "Device id: " << device->getElementId() << endl;
  cout << "Described as: " << device->getElementDescription() << endl;
  cout << endl;
  print(device->getDeviceElementGroup(), ELEMENT_OFFSET);
}

// NOLINTNEXTLINE(cert-err58-cpp)
const static vector<string> device_ids{"base_id_1", "base_id_2"};

Information_Model::NonemptyDevicePtr buildDevice1() {
  auto mock_builder =
      make_shared<Information_Model::testing::DeviceMockBuilder>();

  mock_builder->buildDeviceBase(device_ids[0], "Example 1",
      "This is an example temperature sensor system");
  { // Power group
    auto subgroup_1_ref_id = mock_builder->addDeviceElementGroup(
        "Power", "Groups information regarding the power supply");
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Power",
        "Indicates if system is running on batter power",
        Information_Model::DataType::BOOLEAN,
        []() -> DataVariant { return true; });
    auto subgroup_2_ref_id =
        mock_builder->addDeviceElementGroup(subgroup_1_ref_id, "State",
            "Groups information regarding the power supply");
    mock_builder->addReadableMetric(subgroup_2_ref_id, "Error",
        "Indicates the current error message, regarding power supply",
        Information_Model::DataType::STRING, []() -> DataVariant {
          return string("Main Power Supply Interrupted");
        });
    mock_builder->addWritableMetric(
        subgroup_2_ref_id, "Reset Power Supply",
        "Resets power supply and any related error messages",
        Information_Model::DataType::BOOLEAN,
        [](const DataVariant&) { /*There is nothing to do*/ },
        []() -> DataVariant { return false; });
  }
  mock_builder->addReadableMetric("Temperature",
      "Current measured temperature value in °C",
      Information_Model::DataType::DOUBLE,
      []() -> DataVariant { return (double)20.1; }); // NOLINT

  return Information_Model::NonemptyDevicePtr(mock_builder->getResult());
}

Information_Model::NonemptyDevicePtr buildDevice2() {
  auto mock_builder =
      make_shared<Information_Model::testing::DeviceMockBuilder>();

  mock_builder->buildDeviceBase(device_ids[1], "Example 2",
      "This is an example power measurement sensor system");
  { // Phase 1 group
    auto subgroup_1_ref_id = mock_builder->addDeviceElementGroup(
        "Phase 1", "Groups first phase's power measurements");
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Voltage",
        "Current measured phase voltage in V",
        Information_Model::DataType::DOUBLE, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)239.1;
        });
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Current",
        "Current measured phase current in A",
        Information_Model::DataType::DOUBLE, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)8.8;
        });
  }
  { // Phase 2 group
    auto subgroup_1_ref_id = mock_builder->addDeviceElementGroup(
        "Phase 2", "Groups second phase's power measurements");
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Voltage",
        "Current measured phase voltage in V",
        Information_Model::DataType::DOUBLE, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)239.1;
        });
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Current",
        "Current measured phase current in A",
        Information_Model::DataType::DOUBLE, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)8.8;
        });
  }
  { // Phase 3 group
    auto subgroup_1_ref_id = mock_builder->addDeviceElementGroup(
        "Phase 3", "Groups third phase's power measurements");
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Voltage",
        "Current measured phase voltage in V",
        Information_Model::DataType::DOUBLE, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)239.1;
        });
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Current",
        "Current measured phase current in A",
        Information_Model::DataType::DOUBLE, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)8.8;
        });
  }
  return Information_Model::NonemptyDevicePtr(mock_builder->getResult());
}

void registerDevice(const Information_Model::NonemptyDevicePtr& device,
    const shared_ptr<EventSourceFake>& event_source) {
  print(device);

  event_source->sendEvent(std::make_shared<ModelRepositoryEvent>(device));
}

void registerDevices(const shared_ptr<EventSourceFake>& event_source) {
  registerDevice(buildDevice1(), event_source);
  registerDevice(buildDevice2(), event_source);
}

void deregisterDevices(const shared_ptr<EventSourceFake>& event_source) {
  for (const auto& device_id : device_ids) {
    cout << "Deregistrating device: " << device_id << endl;
    event_source->sendEvent( // deregistrade first device
        std::make_shared<ModelRepositoryEvent>(device_id));
    this_thread::sleep_for(5s);
  }
}