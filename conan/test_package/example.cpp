#include "Event_Model/EventSource.hpp"
#include "LoggerRepository.hpp"
#include "Open62541_Data_Consumer_Adapter/OpcuaAdapter.hpp"

#include <memory>
#include <thread>

using namespace std;
using namespace DCAI;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
  void handleException(exception_ptr eptr) {
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

int main() {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, true, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);
  auto adapter = make_unique<OpcuaAdapter>(make_shared<EventSourceFake>());
  adapter->start();
  this_thread::sleep_for(chrono::seconds(2));
  adapter->stop();
  exit(EXIT_SUCCESS);
}