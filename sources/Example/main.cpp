#include "Open62541Adapter.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <Information_Model_Mocks/MockBuilder.hpp>
#include <Information_Model_Mocks/ReadableMock.hpp>
#include <Variant_Visitor/Visitor.hpp>

#include <chrono>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>
#include <thread>
#include <unordered_map>

using namespace std;
using namespace HaSLL;
using namespace Information_Model;
using namespace Data_Consumer_Adapter;

void printException(
    const LoggerPtr& logger, const exception& e, int level = 0) {
  logger->error("{:level&} Exception: {}", " ", e.what());
  try {
    rethrow_if_nested(e);
  } catch (const exception& nested_exception) {
    printException(logger, nested_exception, level + 1);
  } catch (...) {
    logger->critical("Caught an unknown exception");
  }
}

struct EventSource {
  DataConnectionPtr connect(const DataNotifier& notifier) {
    auto connection = make_shared<Connection>(notifier);
    connection_ = connection;
    return connection;
  }

  void notify(const RegistryChangePtr& event) {
    if (auto locked = connection_.lock()) {
      locked->call(event);
    }
  }

  void registerDevice(const DevicePtr& device) {
    auto event = std::make_shared<RegistryChange>(device);
    notify(event);
  }

  void deregisterDevice(const string& identifier) {
    auto event = std::make_shared<RegistryChange>(identifier);
    notify(event);
  }

private:
  struct Connection : DataConnection {
    Connection(const DataNotifier& notifier) : notify_(notifier) {}

    void call(const RegistryChangePtr& event) { notify_(event); }

  private:
    DataNotifier notify_;
  };

  using WeakConnectionPtr = weak_ptr<Connection>;
  WeakConnectionPtr connection_;
};

using EventSourcePtr = shared_ptr<EventSource>;

void registerDevices(const EventSourcePtr& event_source);
void deregisterDevices(const EventSourcePtr& event_source);

int main(int argc, char* argv[]) {
  auto status = EXIT_SUCCESS;
  try {
    LoggerManager::initialise(
        makeDefaultRepository("config/loggerConfig.json"));
    auto logger = LoggerManager::registerLogger("Main");
    try {
      logger->trace("Logging completed initialization!");

      auto event_source = make_shared<EventSource>();
      logger->trace("Fake event source initialized!");
      auto connector =
          bind(&EventSource::connect, event_source, placeholders::_1);
      auto adapter =
          makeOpen62541Adapter(connector, "config/defaultConfig.json");
      logger->trace("OPC UA Adapter initialized!");

      adapter->start();
      registerDevices(event_source);
      this_thread::sleep_for(10s);
      logger->trace("Sending device deregistered event");
      deregisterDevices(event_source);
      this_thread::sleep_for(5s);
      registerDevices(event_source);

      if (argc > 1) {
        auto server_lifetime = stoi(argv[1]);
        cout << "Open62541 server will automatically shut down in "
             << server_lifetime << " seconds." << endl;
        this_thread::sleep_for(chrono::seconds(server_lifetime));
        adapter->stop();
      } else {
        string user_input;
        do {
          cout << "Press Q to stop the server" << endl;
          cin >> user_input;
          transform(user_input.begin(), user_input.end(), user_input.begin(),
              [](unsigned char letter) { return tolower(letter); });
        } while (user_input != "q");
        adapter->stop();
      }
    } catch (const exception& ex) {
      printException(logger, ex);
      status = EXIT_FAILURE;
    } catch (...) {
      logger->error("Unknown error occurred during program execution");
      status = EXIT_FAILURE;
    }
    LoggerManager::terminate();
  } catch (...) {
    cerr << "Unknown error occurred during program execution or logger "
            "acquisition"
         << endl;
    status = EXIT_FAILURE;
  }
  exit(status);
}

void print(const DevicePtr& device);
void print(const ElementPtr& element, size_t offset);
void print(const WritablePtr& element, size_t offset);
void print(const ReadablePtr& element, size_t offset);
void print(const GroupPtr& elements, size_t offset);

static constexpr size_t BASE_OFFSET = 160;
static constexpr size_t ELEMENT_OFFSET = 3;

void print(const GroupPtr& elements, size_t offset) {
  cout << string(offset, ' ') << "Group contains elements:" << endl;
  for (const auto& element : elements->asVector()) {
    print(element, offset + ELEMENT_OFFSET);
  }
}

void print(const ReadablePtr& element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->dataType())
       << " value: " << toString(element->read()) << endl;
  cout << endl;
}

void print(const ObservablePtr& element, size_t offset) {
  cout << string(offset, ' ') << "Observably Reads "
       << toString(element->dataType())
       << " value: " << toString(element->read()) << endl;
  cout << endl;
}

void print(const WritablePtr& element, size_t offset) {
  cout << string(offset, ' ') << "Reads " << toString(element->dataType())
       << " value: " << toString(element->read()) << endl;
  cout << string(offset, ' ') << "Writes " << toString(element->dataType())
       << " value type" << endl;
  cout << endl;
}

void print(const CallablePtr& element, size_t offset) {
  cout << string(offset, ' ') << "Executes " << toString(element->resultType())
       << " (" << toString(element->parameterTypes()) << ")" << endl;
}

void print(const ElementPtr& element, size_t offset) {
  cout << string(offset, ' ') << "Element name: " << element->name() << endl;
  cout << string(offset, ' ') << "Element id: " << element->id() << endl;
  cout << string(offset, ' ') << "Described as: " << element->description()
       << endl;

  Variant_Visitor::match(
      element->function(),
      [offset](const GroupPtr& group) { print(group, offset); },
      [offset](const ReadablePtr& readable) { print(readable, offset); },
      [offset](const ObservablePtr& observable) { print(observable, offset); },
      [offset](const WritablePtr& writable) { print(writable, offset); },
      [offset](const CallablePtr& executable) { print(executable, offset); });
}

void print(const DevicePtr& device) {
  cout << "Device name: " << device->name() << endl;
  cout << "Device id: " << device->id() << endl;
  cout << "Described as: " << device->description() << endl;
  cout << endl;
  print(device->group(), ELEMENT_OFFSET);
}

const static vector<string_view> device_ids{"base_id_1", "base_id_2"};

// NOLINTBEGIN(readability-magic-numbers)
DevicePtr buildDevice1() {
  auto mock_builder = make_shared<Information_Model::testing::MockBuilder>();

  mock_builder->setDeviceInfo(string{device_ids[0]},
      {"Example 1", "This is an example temperature sensor system"});
  { // Power group
    auto subgroup_1_ref_id = mock_builder->addGroup(
        {"Power", "Groups information regarding the power supply"});
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Power", "Indicates if system is running on batter power"}, true);
    auto subgroup_2_ref_id = mock_builder->addGroup(subgroup_1_ref_id,
        {"State", "Groups information regarding the power supply"});
    mock_builder->addReadable(subgroup_2_ref_id,
        {"Error",
            "Indicates the current error message, regarding power supply"},
        "Main Power Supply Interrupted");
    mock_builder->addWritable(subgroup_2_ref_id,
        {"Reset Power Supply",
            "Resets power supply and any related error messages"},
        DataType::Boolean, [](const DataVariant&) {
          cout << "Reseting power supply for " << string{device_ids[0]} << endl;
        });
  }
  mock_builder->addReadable(
      {"Temperature", "Current measured temperature value in °C"}, 20.1);

  return mock_builder->result();
}

DevicePtr buildDevice2() {
  auto mock_builder = make_shared<Information_Model::testing::MockBuilder>();

  mock_builder->setDeviceInfo(string{device_ids[1]},
      {"Example 2", "This is an example power measurement sensor system"});
  { // Phase 1 group
    auto subgroup_1_ref_id = mock_builder->addGroup(
        {"Phase 1", "Groups first phase's power measurements"});
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Voltage", "Current measured phase voltage in V"}, 241.1);
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Current", "Current measured phase current in A"}, 3.4);
  }
  { // Phase 2 group
    auto subgroup_1_ref_id = mock_builder->addGroup(
        {"Phase 2", "Groups second phase's power measurements"});
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Voltage", "Current measured phase voltage in V"}, 222.1);
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Current", "Current measured phase current in A"}, 6.6);
  }
  { // Phase 3 group
    auto subgroup_1_ref_id = mock_builder->addGroup(
        {"Phase 3", "Groups third phase's power measurements"});
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Voltage", "Current measured phase voltage in V"}, 239.1);
    mock_builder->addReadable(subgroup_1_ref_id,
        {"Current", "Current measured phase current in A"}, 8.8);
  }
  mock_builder->addCallable({"Recalculate", "Recalculates measured values"},
      Information_Model::DataType::Boolean);

  mock_builder->addCallable({"Reset", "Resets the device"},
      [](const Parameters&) { cout << "Callback called" << endl; });
  return mock_builder->result();
}
// NOLINTEND(readability-magic-numbers)

void registerDevice(
    const DevicePtr& device, const EventSourcePtr& event_source) {
  // print(device);

  event_source->registerDevice(device);
}

void registerDevices(const EventSourcePtr& event_source) {
  registerDevice(buildDevice1(), event_source);
  registerDevice(buildDevice2(), event_source);
}

void deregisterDevices(const EventSourcePtr& event_source) {
  for (const auto& device_id : device_ids) {
    cout << "Deregistrating device: " << device_id << endl;
    event_source->deregisterDevice(string{device_id});
    this_thread::sleep_for(5s);
  }
}
