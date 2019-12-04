#ifndef __LISTENER_MOCK_HPP
#define __LISTENER_MOCK_HPP

#include "gmock/gmock.h"

#include "Listener.hpp"
#include "Notifier_Event.hpp"

class MockListener : public Model_Event_Handler::Listener {
public:
  MOCK_METHOD(void, handleEvent, (Model_Event_Handler::NotifierEvent * event),
              (override));
};

#endif //__LISTENER_MOCK_HPP