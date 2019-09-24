#include "OpcuaAdapter.hpp"
#include <iostream>
#include <memory>
#include <unistd.h>

using namespace std;

#define SERVER_LIFETIME 60

int main() {
  OpcuaAdapter *server = new OpcuaAdapter();

  server->startOpen62541();

  sleep(SERVER_LIFETIME);

  exit(0);
}