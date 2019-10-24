#include "OpcuaAdapter.hpp"

using namespace std;

#define SERVER_LIFETIME 60

int main() {

  OpcuaAdapter *adapter = new OpcuaAdapter();
  adapter->start();

  sleep(SERVER_LIFETIME);

  exit(0);
}