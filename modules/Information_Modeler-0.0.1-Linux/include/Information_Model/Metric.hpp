#ifndef METRIC_HPP
#define METRIC_HPP

#include "DataWrapper.hpp"
#include "NamedElement.hpp"

#include <string>

namespace Information_Model {

  /**
  * @brief A Readonly metric type
  * 
  * @attention This class should be built by Model_Factory::DeviceBuilder only!
  * 
  * @tparam T 
  */
  template<class T>
  class Metric : public NamedElement {
   protected:
    Metric(const std::string REF_ID,
        const std::string NAME,
        const std::string DESC)
        : NamedElement(REF_ID, NAME, DESC) {}

   public:
    DataWrapper<T> getMetricValue();
  };
}   // namespace Information_Model
#endif   //METRIC_HPP