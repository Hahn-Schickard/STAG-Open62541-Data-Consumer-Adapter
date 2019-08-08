#include "OpcuaAdapter.hpp"

#include "Server_Runner.hpp"

using namespace Information_Model;

void OpcuaAdapter::startOpen62541() { start(); }
void OpcuaAdapter::stopOpen62541() { stop(); }
void OpcuaAdapter::handleEvent(Device *device) { addDevice(device); }