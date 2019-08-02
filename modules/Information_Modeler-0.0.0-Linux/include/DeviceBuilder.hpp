#ifndef _DEVICE_FACTORY_HPP
#define _DEVICE_FACTORY_HPP

#include "DeviceElement.hpp"

#include "DeviceImpl.hpp"
#include <memory>

/**
 * @brief This namespace contains all of the implementations of Information_Model classes and 
 * building mechanisms for their instances. It should only be used to create a Device Model and 
 * not as it's representatin. 
 */
namespace Model_Factory
{

/**
 * @brief This class constructs an instance of Model_Factory::DevicImpl as a Infromation_Model::Device
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class DeviceBuilder
{
private:
  std::unique_ptr<Information_Model::Device> device;
  Model_Factory::DeviceElementGroupImpl *  getDeviceElementGroup();
  std::string addDeviceElementToSubgroup(Model_Factory::DeviceElementGroupImpl * parentGroup, const std::string GROUP_REFID, const std::string NAME, const std::string DESC, Information_Model::ElementType type);
  Model_Factory::DeviceElementGroupImpl * getSubelementGroup(Model_Factory::DeviceElementGroupImpl * deviceElementGroup, std::string REFID);
  Model_Factory::DeviceElementGroupImpl * findElementGroup(std::string RefId, Model_Factory::DeviceElementGroupImpl * deviceElementGroup);

public:
  DeviceBuilder(const std::string NAME, const std::string REF_ID,
                const std::string DESC);

  /**
   * @brief This calls DeviceImpl::addDeviceElementGroup() method, whcih sets a
   * single instance of Information_Model::DeviceElementGroup within. This method 
   * should be only called once per device construction! 
   * 
   * If called more than once, it will override the previous instance of 
   * DeviceImpl: DeviceImpl::device_element_group
   * If you want to add DeviceElementGroup instances, you need to call 
   * DeviceBuilder::addDeviceElement()
   * 
   * @param NAME 
   * @param DESC 
   * @return std::string Reference ID of DeviceImpl: DeviceImpl::device_element_group
   */
  std::string addDeviceElementGroup(const std::string NAME,
                                    const std::string DESC);
  /**
   * @brief This method calls DeviceElementGroupImpl::addDeviceElement() method to 
   * create an instance of Information_Model:DeviceElement class
   * 
   * @param NAME 
   * @param DESC 
   * @param type 
   * @return std::string Reference ID of  Information_Model:DeviceElement instance 
   */
  std::string addDeviceElement(const std::string NAME, const std::string DESC,
                               Information_Model::ElementType type);


  std::string addDeviceElement(const std::string GROUP_REFID, const std::string NAME, const std::string DESC,
                               Information_Model::ElementType type);

  /**
   * @brief This method returns a pointer with full ownership rights. 
   * @attention If this pointer is not saved by the caller, it will be cleaned up by the 
   * Garbadge collector.
   * 
   * @return std::unique_ptr<Information_Model::Device> 
   */
  std::unique_ptr<Information_Model::Device> getDevice();



};
} // namespace Model_Factory

#endif //_DEVICE_FACTORY_HPP