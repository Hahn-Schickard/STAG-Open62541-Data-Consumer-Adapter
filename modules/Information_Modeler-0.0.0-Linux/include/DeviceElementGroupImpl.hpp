#ifndef _DEVICE_ELEMENT_GROUP_HPP
#define _DEVICE_ELEMENT_GROUP_HPP

#include "DeviceElement.hpp"
#include "DeviceElementGroup.hpp"
#include <memory>
#include <unordered_map>

namespace Model_Factory
{
/**
 * @brief This class is an implementation of Information_Model::DeviceElementGroup
 * It contains all of the virtual method implementations and the building mechanism 
 * for Information_Model::DeviceElementGroup instances. 
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class DeviceElementGroupImpl : public Information_Model::DeviceElementGroup
{
private:
  std::unordered_map<std::string, std::shared_ptr<Information_Model::DeviceElement>> subelements;
  unsigned int elementId;
  Information_Model::DeviceElement * findSubelement(const std::string REF_ID);

public:
  DeviceElementGroupImpl(const std::string refId, const std::string name,
                         const std::string desc);
  /**
   * @brief This method creates an instance of Information_Model:DeviceElement class and
   * adds it to DeviceElementGroupImpl: DeviceElementGroupImpl::subelements vector
   * 
   * @param NAME 
   * @param DESC 
   * @param type 
   * @return std::string Reference ID of DeviceElement within DeviceElementGroupImpl: DeviceElementGroupImpl::subelements vector
   */
  std::string addDeviceElement(const std::string name, const std::string desc,
                               Information_Model::ElementType type);

  std::vector<std::shared_ptr<Information_Model::DeviceElement>> getSubelements();
  Information_Model::DeviceElement *getSubelement(const std::string refId);

  void incrementElementId();

  std::string generate_Reference_ID();

  unsigned int getNumericElementId();

  virtual ~DeviceElementGroupImpl() = default;
};
} // namespace Model_Factory 

#endif //_DEVICE_ELEMENT_GROUP_HPP