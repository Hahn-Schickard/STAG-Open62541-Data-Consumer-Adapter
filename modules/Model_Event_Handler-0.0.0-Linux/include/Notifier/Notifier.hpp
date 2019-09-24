#ifndef __NOTIFIER_HPP
#define __NOTIFIER_HPP

#include "Notifier_Event.hpp"
#include "Listener.hpp"
#include <memory>
#include <vector>

namespace Notifier
{
class Notifier
{
public:
  void registerListener(std::reference_wrapper<Listener> listiner)
  {
    listiners.push_back(listiner);
  }

protected:
  void notifyListeners(NotifierEvent *event)
  {
    for (auto listiner : listiners)
    {
      listiner.get().handleEvent(event);
    }
  }

  std::vector<std::reference_wrapper<Listener>> listiners;
};
} // namespace Notifier

#endif //__NOTIFIER_HPP