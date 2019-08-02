#ifndef __NOTIFIER_HPP
#define __NOTIFIER_HPP

#include "Listener.hpp"
#include <memory>

namespace Notifier {
class Notifier {
public:
  void registerListener(std::shared_ptr<Listener> listiner) {
    listiners.push_back(listiner);
  }

protected:
  void notifyListeners(Information_Model::Device *device) {
    for (auto listiner : listiners) {
      listiner->handleEvent(device);
    }
  }

private:
  std::vector<std::shared_ptr<Listener>> listiners;
};
} // namespace Notifier

#endif //__NOTIFIER_HPP