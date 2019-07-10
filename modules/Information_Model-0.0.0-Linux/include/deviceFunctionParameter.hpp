#ifndef FUNCTIONPARAMETER
#define FUNCTIONPARAMETER

#include "valueDataType.hpp"
#include "namedElement.hpp"
#include <string>



class FunctionParameter : public NamedElement
{
private:
	bool isOptionalFlag;
	ValueType::ValueDataType dataType;

public:
	FunctionParameter(std::string refId, std::string name, std::string desc, ValueType::ValueDataType valueType);

	FunctionParameter(std::string refId, std::string name, std::string desc, ValueType::ValueDataType valueType, bool isOptional);

	bool isOptional();

	ValueType::ValueDataType getDataType();
};
#endif // !FUNCTIONPARAMETER
