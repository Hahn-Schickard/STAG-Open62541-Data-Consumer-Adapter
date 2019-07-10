#ifndef DEVICEELEMENT_H
#define DEVICEELEMENT_H

#include "namedElement.hpp"

enum ElementType {
	Undefined,
	Group,
	Readonly,
	Observable,
	Writable,
	Function
};

class DeviceElement : public NamedElement
{

public:
	ElementType elementType = Undefined;
	
	DeviceElement(std::string refId, std::string name, std::string desc) ;

	ElementType getElementType();
};


#endif // !DEVICEELEMENT_H
