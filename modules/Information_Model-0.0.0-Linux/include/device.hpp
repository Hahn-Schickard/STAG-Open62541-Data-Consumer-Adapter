#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>

#include "deviceElementGroup.hpp"
#include "namedElement.hpp"

class Device : public NamedElement
{
private:
	std::vector<DeviceElementGroup*> device_element_groups;
	
public:
	Device(std::string refId, std::string name, std::string desc);
	


	void addDeviceElementGroup(DeviceElementGroup * group);

	DeviceElementGroup * getDeviceElementGroup();
	DeviceElementGroup * getDeviceElementGroup(std::string refId);
	std::vector<DeviceElementGroup*> getDeviceElementGroups();
	DeviceElementGroup* findSubElementGroup(DeviceElementGroup* deviceElementGroup, std::string refId);
	DeviceElement* findSubElement(DeviceElementGroup* deviceElementGroup, std::string refId);

	
};

#endif // !DEVICE_H


