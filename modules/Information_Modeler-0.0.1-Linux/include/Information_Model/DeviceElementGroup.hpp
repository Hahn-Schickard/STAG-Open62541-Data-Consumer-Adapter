#ifndef DEVICE_ELEMENT_GROUP_HPP
#define DEVICE_ELEMENT_GROUP_HPP

#include "DeviceElement.hpp"
#include "NamedElement.hpp"
#include <memory>
#include <string>
#include <vector>



namespace Information_Model
{

/**
 * @brief This Class Models a List of Elements within the Device. 
 * It may contain more DeviceElementGroup instances, which act as sublists. 
 * The implementation of this class exists in Model_Factory::DeviceElementGroupImpl
 * @attention This class should be instantiated by Model_Factory::DeviceBuilder only!
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
  class DeviceElementGroup : public DeviceElement
  {
  protected:
    DeviceElementGroup(const std::string REF_ID, const std::string NAME,
                      const std::string DESC)
        : DeviceElement(REF_ID, NAME, DESC, ElementType::Group) {
          
        }

    

  public:
    virtual std::vector<std::shared_ptr<DeviceElement>> getSubelements() = 0;

  

    //virtual DeviceElement *getSubelement(const std::string REF_ID) = 0; //bearbeitet
    virtual std::shared_ptr<DeviceElement>
    getSubelement(const std::string REF_ID) = 0;
    virtual unsigned int getNumericElementId() = 0;

    virtual ~DeviceElementGroup() = default;

  };
} // namespace Information_Model

#endif //DEVICE_ELEMENT_GROUP_HPP