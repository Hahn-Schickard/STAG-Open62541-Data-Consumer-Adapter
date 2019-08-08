#ifndef __SERVER_RUNNER_H
#define __SERVER_RUNNER_H

#include "Device.hpp"

bool start();
bool stop();
void addDevice(Information_Model::Device *device);

#endif //__SERVER_RUNNER_H