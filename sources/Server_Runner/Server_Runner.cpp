#include "Server_Runner.hpp"
#include "Open62541_Server.hpp"

using namespace std;
using namespace Information_Model;

bool start() { return startServer(); }
bool stop() { return stopServer(); }
void addDevice(shared_ptr<Device> device) { addDeviceNode(device); }