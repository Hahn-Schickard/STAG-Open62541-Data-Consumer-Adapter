#ifndef __LISTENER__HPP
#define __LISTENER__HPP

#include "Device.hpp"

namespace Notifier {
class Listener {
public:
  virtual ~Listener() = default;
  virtual void handleEvent(Information_Model::Device *device) = 0;
};
} // namespace Notifier

#endif //__LISTENER__HPP