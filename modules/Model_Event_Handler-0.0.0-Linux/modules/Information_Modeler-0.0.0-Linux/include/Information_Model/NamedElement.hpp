#ifndef _NAMEDELEMENT_HPP
#define _NAMEDELEMENT_HPP

#include <string>
/**
 * @brief This Namespace contains all of the definitions of modelling classes,
 * most of which act only as an interface.
 * @attention None of these classes should be built by calling their
 * constructors derectlly.
 * The construction of the modelling classes is handled by the
 * Model_Factory::DeviceBuilder.
 */
namespace Information_Model {
/**
 * @brief This is the base class for all of the other modelling classes.
 * @attention This class should be instantiated by its children only!
 * @author Dovydas Girdvainis
 * @date 18.07.2019
 */
class NamedElement {
private:
  std::string name;
  std::string desc;
  std::string refId;

protected:
  NamedElement(const std::string REF_ID, const std::string NAME,
               const std::string DESC) {
    name = NAME;
    desc = DESC;
    refId = REF_ID;
  }

public:
  const std::string getElementName() { return name; }
  const std::string getElementDescription() { return desc; }
  const std::string getElementRefId() { return refId; }

  virtual ~NamedElement() = default;
};
} // namespace Information_Model

#endif //_NAMEDELEMENT_HPP