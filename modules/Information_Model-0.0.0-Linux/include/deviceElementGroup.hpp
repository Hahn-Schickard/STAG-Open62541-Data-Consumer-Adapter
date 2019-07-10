#ifndef DEVICEELEMENTGROUP_H
#define DEVICEELEMENTGROUP_H


#include "deviceElement.hpp"
#include <algorithm>
#include <vector>
#include <string>

class DeviceElementGroup : public DeviceElement
{
protected:
	std::vector<DeviceElement*> subElements;
	
public:
	DeviceElementGroup(std::string refId, std::string name, std::string desc);
	std::vector<DeviceElement*> getSubElements();
	std::vector<DeviceElementGroup*>getSubElementGroups();

	void addElement(DeviceElement &element);
	DeviceElement* GetElementByRefId(std::string refId)
	{
		auto elem = std::find_if(subElements.begin(), subElements.end(), [refId](DeviceElement* obj) { return obj->getReferenceId() == refId; });
		if (elem != subElements.end())
		{
			return *elem;
		}
		return *elem;
	}


};

#endif // !DEVICEELEMENTGROUP_H


