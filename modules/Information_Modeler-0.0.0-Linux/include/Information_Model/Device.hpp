#ifndef _I_DEVICE_HPP
#define _I_DEVICE_HPP

#include "DeviceElementGroup.hpp"
#include "NamedElement.hpp"
#include <string>


namespace Information_Model {
/**
 * @brief This class models a Device.
 * It contains a single instance of Information_Model::DeviceElementGroup, which
 * has all of the elements that this device uses.
 * @attention This class should be built by Model_Factory::DeviceBuilder only!
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class Device : public NamedElement {
protected:
  Device(const std::string REF_ID, const std::string NAME,
         const std::string DESC)
      : NamedElement(REF_ID, NAME, DESC) {}

public:
  //virtual DeviceElementGroup *getDeviceElementGroup() = 0; //bearbeitet
  virtual std::shared_ptr<Information_Model::DeviceElementGroup> getDeviceElementGroup() = 0; 
};
} // namespace Information_Model

#endif //_I_DEVICE_HPP