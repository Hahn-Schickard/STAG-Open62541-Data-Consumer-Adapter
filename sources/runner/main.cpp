#include "LoggerRepository.hpp"
#include "OpcuaAdapter.hpp"

#include <iostream>
#include <signal.h>

using namespace std;
using namespace HaSLL;

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

int main(int argc, char *argv[]) {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);
  signal(SIGINT, stopHandler);
  signal(SIGTERM, stopHandler);

  adapter = new OpcuaAdapter();
  adapter->start();

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