#include "Event_Model/EventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "Information_Model/mocks/Metric_MOCK.hpp"

#include "OpcuaAdapter.hpp"

#include <chrono>
#include <csignal>
#include <exception>
#include <functional>
#include <gmock/gmock.h>
#include <iostream>
#include <thread>
#include <unordered_map>

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

void printException(
    const LoggerPtr& logger, const exception& e, int level = 0) {
  logger->error("{:level&} Exception: {}", " ", e.what());
  try {
    rethrow_if_nested(e);
  } catch (const exception& nested_exception) {
    printException(logger, nested_exception, level + 1);
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

struct Executor {
  using Callback = function<void(void)>;
  Executor(Callback&& callback) : callback_(move(callback)) {}

  pair<uintmax_t, future<DataVariant>> execute(const Function::Parameters&) {
    auto execute_lock = lock_guard(execute_mx_);
    auto id = calls_.size();
    auto promise = std::promise<DataVariant>();
    auto future = make_pair(id, promise.get_future());
    calls_.emplace(id, move(promise));
    auto responder = bind(&Executor::respond, this, id);
    thread(
        [](function<void(void)>&& responder) {
          this_thread::sleep_for(1s);
          if (responder) {
            responder();
          }
        },
        move(responder))
        .detach();

    return future;
  }

  void cancel(uintmax_t id) {
    auto iter = calls_.find(id);
    if (iter != calls_.end()) {
      iter->second.set_exception(
          make_exception_ptr(CallCanceled(id, "ExternalExecutor")));
      auto clear_lock = lock_guard(erase_mx_);
      iter = calls_.erase(iter);
    } else {
      throw CallerNotFound(id, "ExternalExecutor");
    }
  }

private:
  void respond(uintmax_t id) {
    auto erase_lock = lock_guard(erase_mx_);
    auto it = calls_.find(id);
    if (it != calls_.end()) {
      if (callback_) {
        callback_();
        it->second.set_value(DataVariant());
      } else {
        it->second.set_exception(
            make_exception_ptr(runtime_error("Callback dos not exist")));
      }
      it = calls_.erase(it);
    }
  }

  Callback callback_;
  mutex execute_mx_;
  mutex erase_mx_;
  unordered_map<uintmax_t, promise<DataVariant>> calls_;
};

using ExecutorPtr = shared_ptr<Executor>;

ExecutorPtr executor = nullptr;

void print(const NonemptyDevicePtr& device);
void print(const NonemptyDeviceElementPtr& element, size_t offset);
void print(const NonemptyWritableMetricPtr& element, size_t offset);
void print(const NonemptyMetricPtr& element, size_t offset);
void print(const NonemptyDeviceElementGroupPtr& elements, size_t offset);

void registerDevices(const shared_ptr<EventSourceFake>& event_source);
void deregisterDevices(const shared_ptr<EventSourceFake>& event_source);

int main(int argc, char* argv[]) {
  auto status = EXIT_SUCCESS;
  try {
    LoggerManager::initialise(
        makeDefaultRepository("config/loggerConfig.json"));
    auto logger = LoggerManager::registerLogger("Main");
    try {
      logger->trace("Logging completed initialization!");
      logger->info(
          "Current Sever time, in number of 100 nanosecond intervals since "
          "January 1, 1601 (UTC): {}",
          UA_DateTime_now());

      signal(SIGINT, stopHandler); // NOLINT(cert-err33-c)
      signal(SIGTERM, stopHandler); // NOLINT(cert-err33-c)

      logger->trace(
          "Termination and Interruption signals assigned to stop handler!");

      auto event_source = make_shared<EventSourceFake>();
      logger->trace("Fake event source initialized!");

      adapter =
          make_unique<OpcuaAdapter>(event_source, "config/defaultConfig.json");
      logger->trace("OPC UA Adapter initialized!");

      executor =
          make_shared<Executor>([]() { cout << "Callback called" << endl; });

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
        stopServer();
      } else {
        while (true) {
          this_thread::sleep_for(1s);
        }
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

void print(const NonemptyObservableMetricPtr& element, size_t offset) {
  cout << string(offset, ' ') << "Observably Reads "
       << toString(element->getDataType())
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
  cout << string(offset, ' ') << "Executes " << toString(element->resultType())
       << " (" << toString(element->parameterTypes()) << ")" << endl;
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
      [offset](
          const NonemptyDeviceElementGroupPtr& group) { print(group, offset); },
      [offset](const NonemptyMetricPtr& readable) { print(readable, offset); },
      [offset](const NonemptyObservableMetricPtr& observable) {
        print(observable, offset);
      },
      [offset](const NonemptyWritableMetricPtr& writable) {
        print(writable, offset);
      },
      [offset](const NonemptyFunctionPtr& executable) {
        print(executable, offset);
      });
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
        Information_Model::DataType::Boolean,
        []() -> DataVariant { return true; });
    auto subgroup_2_ref_id =
        mock_builder->addDeviceElementGroup(subgroup_1_ref_id, "State",
            "Groups information regarding the power supply");
    mock_builder->addReadableMetric(subgroup_2_ref_id, "Error",
        "Indicates the current error message, regarding power supply",
        Information_Model::DataType::String, []() -> DataVariant {
          return string("Main Power Supply Interrupted");
        });
    mock_builder->addWritableMetric(
        subgroup_2_ref_id, "Reset Power Supply",
        "Resets power supply and any related error messages",
        Information_Model::DataType::Boolean,
        [](const DataVariant&) { /*There is nothing to do*/ },
        []() -> DataVariant { return false; });
  }
  mock_builder->addReadableMetric("Temperature",
      "Current measured temperature value in °C",
      Information_Model::DataType::Double,
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
        Information_Model::DataType::Double, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)239.1;
        });
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Current",
        "Current measured phase current in A",
        Information_Model::DataType::Double, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)8.8;
        });
  }
  { // Phase 2 group
    auto subgroup_1_ref_id = mock_builder->addDeviceElementGroup(
        "Phase 2", "Groups second phase's power measurements");
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Voltage",
        "Current measured phase voltage in V",
        Information_Model::DataType::Double, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)239.1;
        });
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Current",
        "Current measured phase current in A",
        Information_Model::DataType::Double, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)8.8;
        });
  }
  { // Phase 3 group
    auto subgroup_1_ref_id = mock_builder->addDeviceElementGroup(
        "Phase 3", "Groups third phase's power measurements");
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Voltage",
        "Current measured phase voltage in V",
        Information_Model::DataType::Double, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)239.1;
        });
    mock_builder->addReadableMetric(subgroup_1_ref_id, "Current",
        "Current measured phase current in A",
        Information_Model::DataType::Double, []() -> DataVariant {
          // NOLINTNEXTLINE(readability-magic-numbers)
          return (double)8.8;
        });
  }
  mock_builder->addFunction("Recalculate", "Recalculates measured values",
      Information_Model::DataType::Boolean);

  if (executor) {
    mock_builder->addFunction("Reset", "Resets the device",
        Information_Model::DataType::None,
        bind(&Executor::execute, executor, placeholders::_1),
        bind(&Executor::cancel, executor, placeholders::_1));
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