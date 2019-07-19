#ifndef __CONSUMER_NOTIFIER_HPP
#define __CONSUMER_NOTIFIER_HPP

#include "NodeInformation.h"

extern void update_DataConsumer(NodeDescription *device);

void notifyConsumers(NodeDescription *device);

#endif //__CONSUMER_NOTIFIER_HPP