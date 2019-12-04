#ifndef __DEVICEELEMENTGROUP_MOCK_HPP
#define __DEVICEELEMENTGROUP_MOCK_HPP

#include "gmock/gmock.h"

#include "DeviceElementGroup.hpp"

class MockDeviceElementGroup : public Information_Model::DeviceElementGroup {
public:
  MockDeviceElementGroup(const std::string REF_ID, const std::string NAME,
             const std::string DESC)
      : Information_Model::DeviceElementGroup(REF_ID, NAME, DESC) {}
  MOCK_METHOD(std::vector<std::shared_ptr<DeviceElement>>, getSubelements, (),
              (override));
  //MOCK_METHOD(Information_Model::DeviceElement *, getSubelement,  //bearbeitet
  //            (const std::string REF_ID), (override));
  MOCK_METHOD(std::shared_ptr<Information_Model::DeviceElement>, getSubelement,
              (const std::string REF_ID), (override));
  MOCK_METHOD(unsigned int, getNumericElementId, (), (override));
};

#endif //__DEVICEELEMENTGROUP_MOCK_HPP