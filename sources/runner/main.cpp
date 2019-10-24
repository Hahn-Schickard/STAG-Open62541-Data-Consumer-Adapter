#include "OpcuaAdapter.hpp"

#include <iostream>
#include <signal.h>

using namespace std;

OpcuaAdapter *adapter;

void stopServer() {
  adapter->stop();
  delete adapter;
}

static void stopHandler(int sig) {
  cout << "Received stop signal!" << endl;
  stopServer();
}

int main(int argc, char *argv[]) {
  signal(SIGINT, stopHandler);
  signal(SIGTERM, stopHandler);

  adapter = new OpcuaAdapter();
  adapter->start();

  cout << "Started open62541 server!" << endl
       << "Press Ctrl+C to stop the program!" << endl;

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

  exit(0);
}