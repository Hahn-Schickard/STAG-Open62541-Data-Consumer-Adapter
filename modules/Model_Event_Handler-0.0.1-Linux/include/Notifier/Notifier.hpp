#ifndef __NOTIFIER_HPP
#define __NOTIFIER_HPP

#include "Listener.hpp"
#include "Notifier_Event.hpp"
#include <memory>
#include <vector>

namespace Model_Event_Handler {
class Notifier {
public:
  void registerListener(std::shared_ptr<Listener> listiner) {
    listiners.push_back(listiner);
  }

protected:
  void notifyListeners(NotifierEvent *event) {
    for (auto listiner : listiners) {
      listiner.get()->handleEvent(event);
    }
  }

  std::vector<std::shared_ptr<Listener>> listiners;
};
} // namespace Model_Event_Handler

#endif //__NOTIFIER_HPP