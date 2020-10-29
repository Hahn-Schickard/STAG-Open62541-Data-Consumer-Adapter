#include "DataConsumerAdapterInterface.hpp"
#include "DeviceMockBuilder.hpp"
#include "LoggerRepository.hpp"
#include "Metric_MOCK.hpp"
#include "OpcuaAdapter.hpp"

#include <gmock/gmock.h>
#include <iostream>
#include <signal.h>

using namespace std;
using namespace HaSLL;
using namespace DCAI;

OpcuaAdapter *adapter;

void stopServer() {
  adapter->stop();
  delete adapter;
}

static void stopHandler(int sig) {
  cout << "Received stop signal!" << endl;
  stopServer();
  exit(0);
}

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
public:
  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

int main(int argc, char *argv[]) {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);
  signal(SIGINT, stopHandler);
  signal(SIGTERM, stopHandler);

  auto event_source = make_shared<EventSourceFake>();
  adapter = new OpcuaAdapter(event_source);
  adapter->start();

  auto mock_builder =
      make_shared<Information_Model::testing::DeviceMockBuilder>();
  mock_builder->buildDeviceBase("1234", "Mocky", "Mocked test device");
  mock_builder->addDeviceElementGroup("Group 1", "First group");
  auto readable_ref_id = mock_builder->addReadableMetric(
      "Readable", "Mocked readable metric",
      Information_Model::DataType::BOOLEAN, Information_Model::ReadFunctor());
  auto device = mock_builder->getResult();
  mock_builder.reset();

  auto readable = static_pointer_cast<Information_Model::testing::MockMetric>(
      device->getDeviceElement(readable_ref_id));
  ON_CALL(*readable.get(), getMetricValue())
      .WillByDefault(
          ::testing::Return(Information_Model::DataVariant((bool)true)));

  event_source->sendEvent(make_shared<ModelRegistryEvent>(device));

  if (argc > 1) {
    uint server_lifetime = atoi(argv[1]);
    cout << "Open62541 server will automatically shut down in "
         << server_lifetime << " seconds." << endl;
    sleep(server_lifetime);
    stopServer();
  } else {
    while (true)
      ;
  }
}