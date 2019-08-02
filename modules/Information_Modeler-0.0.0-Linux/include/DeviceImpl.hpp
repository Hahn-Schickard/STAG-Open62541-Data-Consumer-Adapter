#ifndef _DEVICE_HPP
#define _DEVICE_HPP

#include <memory>
#include <string>

#include "Device.hpp"
#include "DeviceElementGroupImpl.hpp"
#include "NamedElement.hpp"

namespace Model_Factory
{
/**
 * @brief This class is an implementation of Information_Model::Device
 * It contains all of the virtual method implementations and the building mechanism 
 * for Information_Model::Device instances. 
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class DeviceImpl : public Information_Model::Device
{
private:
  std::unique_ptr<Information_Model::DeviceElementGroup> device_element_group;

public:
  DeviceImpl(const std::string REF_ID, const std::string NAME,
             const std::string DESC)
      : Information_Model::Device(REF_ID, NAME, DESC) {}

  /**
   * @brief This sets the DeviceImpl: DeviceImpl::device_element_group field.
   * @attention. This method should be only called once per device construction! 
   * If called more than once, it will override the previous instance of 
   * DeviceImpl: DeviceImpl::device_element_group
   * 
   * @param NAME 
   * @param DESC 
   * @return std::string Reference ID of DeviceImpl: DeviceImpl::device_element_group
   */
  std::string addDeviceElementGroup(const std::string NAME,
                                    const std::string DESC);
  Information_Model::DeviceElementGroup *getDeviceElementGroup();
};
} // namespace Model_Factory

#endif //_DEVICE_HPP
