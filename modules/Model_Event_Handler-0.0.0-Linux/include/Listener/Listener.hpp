#ifndef __LISTENER__HPP
#define __LISTENER__HPP

#include "Notifier_Event.hpp"

namespace Model_Event_Handler {
class Listener {
public:
  virtual ~Listener() = default;
  virtual void handleEvent(NotifierEvent *event) = 0;
};
} // namespace Model_Event_Handler

#endif //__LISTENER__HPP