#include "Open62541_Data_Consumer_Adapter/OpcuaAdapter.hpp"
#include "Event_Model/EventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"

#include <iostream>
#include <memory>
#include <thread>

using namespace std;
using namespace DCAI;
using namespace HaSLL;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
  void handleException(exception_ptr eptr) {
    if (eptr) {
      rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  void sendEvent(ModelRegistryEventPtr event) { notify(event); }
};

int main() {
  try {
    auto repo = make_shared<SPD_LoggerRepository>();
    LoggerManager::initialise(repo);
    auto logger = LoggerManager::registerLogger("main");
    try {
      auto adapter = make_unique<OpcuaAdapter>(
          make_shared<EventSourceFake>(), "config/defaultConfig.json");
      adapter->start();
      this_thread::sleep_for(chrono::seconds(2));
      adapter->stop();
      logger->info("Integration test succeeded.");
      exit(EXIT_SUCCESS);
    } catch (const exception& ex) {
      logger->error("Integration test failed due to exception: {}", ex.what());
      exit(EXIT_FAILURE);
    }
  } catch (const exception& ex) {
    cerr << "Integration test failed due to exception: " << ex.what() << endl;
    exit(EXIT_FAILURE);
  }
}
