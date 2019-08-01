#ifndef __OPCUA_ADAPTER_HPP
#define __OPCUA_ADAPTER_HPP

#include "ConsumerNotifier.hpp"
#include "Open62541_Server.h"

class OpcuaAdapter : public OPCUA_Notifier::ConsumerNotifier {
public:
  void startOpen62541();
  void stopOpen62541();
};

#endif //__OPCUA_ADAPTER_HPP