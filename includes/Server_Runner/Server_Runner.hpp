#ifndef __SERVER_RUNNER_H
#define __SERVER_RUNNER_H

#include "Device.hpp"
#include <memory>

bool start();
bool stop();
void addDevice(std::shared_ptr<Information_Model::Device> device);

#endif //__SERVER_RUNNER_H