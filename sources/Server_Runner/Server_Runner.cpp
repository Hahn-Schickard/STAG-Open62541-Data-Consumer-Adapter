#include "Server_Runner.hpp"
#include "Open62541_Server.hpp"

bool start() { return startServer(); }
bool stop() { return stopServer(); }
void addDevice(Information_Model::Device *device) { addDeviceNode(device); }