#include "Event_Model/EventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "Open62541_Data_Consumer_Adapter/OpcuaAdapter.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <thread>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace HaSLL;

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
  void handleException(exception_ptr eptr) {
    if (eptr) {
      rethrow_exception(eptr);
    }
  }

public:
  EventSourceFake()
      : EventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)) {}

  void sendEvent(ModelRepositoryEventPtr event) { notify(event); }
};

int main() {
  try {
    auto repo = make_shared<SPD_LoggerRepository>();
    LoggerManager::initialise(repo);
    auto logger = LoggerManager::registerLogger("main");

    auto adapter = make_unique<OpcuaAdapter>(
        make_shared<EventSourceFake>(), "config/defaultConfig.json");

    adapter->start();
    this_thread::sleep_for(chrono::seconds(2));

    adapter->stop();
    logger->info("Integration test succeeded");

    exit(EXIT_SUCCESS);
  } catch (const exception& ex) {
    printException(ex);
    cerr << "Integration test failed" << endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    cerr << "Unknown error occurred during program execution" << endl;
    cerr << "Integration test failed" << endl;
    exit(EXIT_FAILURE);
  }
}
