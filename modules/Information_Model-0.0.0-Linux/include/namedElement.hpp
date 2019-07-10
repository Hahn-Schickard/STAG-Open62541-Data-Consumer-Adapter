#ifndef NAMEDELEMENT_H
#define NAMEDELEMENT_H


#include <string>
#include <vector>

class NamedElement
{
protected:
	std::string Name = "";
	std::string Desc = "";
	std::string RefId = "";

public:
	std::string getElementDescription() { return this->Desc; }
	std::string getElementName() { return this->Name; }
	const std::string getReferenceId() { return this->RefId; }



	NamedElement(std::string refId, std::string name, std::string desc);

	bool ContainsNonNumericCharacters(std::string refId);
	
};

#endif
