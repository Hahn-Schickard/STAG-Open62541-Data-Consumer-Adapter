#ifndef _I_DEVICE_ELEMENT_HPP
#define _I_DEVICE_ELEMENT_HPP

#include "NamedElement.hpp"
#include <string>

namespace Information_Model {

enum ElementType { Undefined, Group, Readonly, Observable, Writable, Function };

/**
 * @brief This class is the base for all of the elements within the
 * Information_Model::Device.
 * @attention This class should be instantiated by its children only!
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class DeviceElement : public NamedElement {
protected:
  ElementType elementType = ElementType::Undefined;

public:
  DeviceElement(const std::string REF_ID, const std::string NAME,
                const std::string DESC, ElementType type)
      : NamedElement(REF_ID, NAME, DESC), elementType(type) {}

  ElementType getElementType() { return elementType; }
  virtual ~DeviceElement() = default;
};
} // namespace Information_Model

#endif //_I_DEVICE_ELEMENT_HPP