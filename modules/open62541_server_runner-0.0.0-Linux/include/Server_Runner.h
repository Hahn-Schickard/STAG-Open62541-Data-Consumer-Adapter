#ifndef __SERVER_RUNNER_H
#define __SERVER_RUNNER_H

#include "NodeInformation.h"

bool start();
bool stop();
void addDevice(NodeDescription *device);

#endif //__SERVER_RUNNER_H