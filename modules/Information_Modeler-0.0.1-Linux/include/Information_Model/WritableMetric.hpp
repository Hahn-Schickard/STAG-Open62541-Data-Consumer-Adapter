#ifndef WRITEABLEMETRIC_HPP
#define WRITEABLEMETRIC_HPP

#include "Metric.hpp"

namespace Information_Model {
  /**
  * @brief A writeable metric type
  * 
  * This metric type also allows read access.
  * 
  * @attention This class should be built by Model_Factory::DeviceBuilder only!
  * 
  * @tparam T 
  */
  template<class T>
  class WritableMetric : public Metric<T> {
   protected:
    WritableMetric(const std::string REF_ID,
        const std::string NAME,
        const std::string DESC)
        : Metric<T>(REF_ID, NAME, DESC) {}

   public:
    void setMetricValue(DataWrapper<T> value);
  };
}   // namespace Information_Model
#endif   //WRITEABLEMETRIC_HPP