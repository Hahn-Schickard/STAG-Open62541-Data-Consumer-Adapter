#ifndef __LISTENER__HPP
#define __LISTENER__HPP

#include "Notifier_Event.hpp"

namespace Notifier
{
class Listener
{
public:
  virtual ~Listener() = default;
  virtual void handleEvent(NotifierEvent *event) = 0;
};
} // namespace Notifier

#endif //__LISTENER__HPP