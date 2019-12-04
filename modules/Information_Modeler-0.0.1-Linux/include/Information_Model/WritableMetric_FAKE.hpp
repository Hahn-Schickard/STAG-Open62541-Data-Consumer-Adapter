#ifndef __WRITABLE_METRIC_FAKE_HPP_
#define __WRITABLE_METRIC_FAKE_HPP_

#include "WritableMetric.hpp"

template<class T>
class FakeWritableMetric : public Information_Model::WritableMetric<T> {
 public:
  FakeWritableMetric(const std::string REF_ID,
      const std::string NAME,
      const std::string DESC,
      T value)
      : Information_Model::WritableMetric<T>(REF_ID, NAME, DESC)
      , value_(Information_Model::DataWrapper<T>(value)) {}

  void setMetricValue(Information_Model::DataWrapper<T> value) {
    value_ = value;
  }

  Information_Model::DataWrapper<T> getMetricValue() {
    return value_;
  }

 private:
  Information_Model::DataWrapper<T> value_;
};

#endif   //__WRITABLE_METRIC_FAKE_HPP_