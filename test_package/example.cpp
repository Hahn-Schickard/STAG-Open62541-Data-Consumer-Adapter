#include "Event_Model/EventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
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

  void sendEvent(ModelRegistryEventPtr event) { notify(event); }
};

int main() {
  auto repo =
      std::make_shared<HaSLL::SPD_LoggerRepository>("config/loggerConfig.json");
  HaSLL::LoggerManager::initialise(repo);

  auto adapter = make_unique<OpcuaAdapter>(
      make_shared<EventSourceFake>(), "config/defaultConfig.json");
  adapter->start();
  this_thread::sleep_for(chrono::seconds(2));
  adapter->stop();
  exit(EXIT_SUCCESS);
}