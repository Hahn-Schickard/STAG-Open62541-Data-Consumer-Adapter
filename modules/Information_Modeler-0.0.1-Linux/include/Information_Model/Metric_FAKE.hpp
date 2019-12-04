#ifndef __METRIC_FAKE_HPP_
#define __METRIC_FAKE_HPP_

#include "Metric.hpp"

template<class T>
class FakeMetric : public Information_Model::Metric<T> {
 public:
  FakeMetric(const std::string REF_ID,
      const std::string NAME,
      const std::string DESC,
      T value)
      : Information_Model::Metric<T>(REF_ID, NAME, DESC)
      , value_(Information_Model::DataWrapper<T>(value)) {}

  Information_Model::DataWrapper<T> getMetricValue() {
    return value_;
  }

 private:
  Information_Model::DataWrapper<T> value_;
};

#endif   //__METRIC_FAKE_HPP_