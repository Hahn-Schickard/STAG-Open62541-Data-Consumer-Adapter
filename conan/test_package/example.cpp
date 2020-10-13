#include "LoggerRepository.hpp"
#include "OpcuaAdapter.hpp"

#include <memory>
#include <thread>

using namespace std;
using namespace DCAI;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
public:
  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

int main() {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);
  auto adapter = make_unique<OpcuaAdapter>(make_shared<EventSourceFake>());
  adapter->start();
  this_thread::sleep_for(chrono::seconds(2));
  adapter->stop();
  exit(EXIT_SUCCESS);
}