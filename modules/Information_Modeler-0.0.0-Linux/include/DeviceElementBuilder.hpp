#ifndef __DEVICE_ELEMENT_BUILDER_HPP
#define __DEVICE_ELEMENT_BUILDER_HPP

#include "DeviceElement.hpp"
#include <memory>
#include <string>

namespace Model_Factory
{

/**
 * @brief This class constructs an instance of Information_Model::DeviceElement that
 * are used by Model_Factory::DeviceBuilder when adding DeviceElement instances into 
 * DeviceElementGroup 
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class DeviceElementBuilder : public Information_Model::DeviceElement
{
public:
  DeviceElementBuilder(const std::string REF_ID, const std::string NAME,
                       const std::string DESC,
                       Information_Model::ElementType type)
      : Information_Model::DeviceElement(REF_ID, NAME, DESC, type) {}
};
} // namespace Model_Factory

#endif //__DEVICE_ELEMENT_BUILDER_HPP