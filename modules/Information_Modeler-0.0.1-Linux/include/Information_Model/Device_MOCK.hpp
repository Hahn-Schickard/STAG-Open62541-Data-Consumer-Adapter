#ifndef __DEVICE_MOCK_HPP
#define __DEVICE_MOCK_HPP

#include "gmock/gmock.h"

#include "Device.hpp"

class MockDevice : public Information_Model::Device {
public:
  MockDevice(const std::string REF_ID, const std::string NAME,
             const std::string DESC)
      : Information_Model::Device(REF_ID, NAME, DESC) {}
  //MOCK_METHOD(Information_Model::DeviceElementGroup *, getDeviceElementGroup,  //bearbeitet
  //            (), (override));
  MOCK_METHOD(std::shared_ptr<Information_Model::DeviceElementGroup> , getDeviceElementGroup,
              (), (override));
};

#endif //__DEVICE_MOCK_HPP
