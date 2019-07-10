#ifndef INFORMATIONMODELEXCEPTONS_H
#define INFORMATIONMODELEXCEPTONS_H
#include <string>

struct InvalidReferenceIdException : public std::exception {
	std::string desc;
	const char * what() const throw ();
	InvalidReferenceIdException() {
		this->desc = "Invalid Reference Id";
	}
	InvalidReferenceIdException(std::string desc)
	{
		this->desc = desc;
	}
};

struct ElementTypeMismatchException : public std::exception {
	std::string desc;
	const char * what() const throw ();
	ElementTypeMismatchException() {}
	ElementTypeMismatchException(std::string desc)
	{
		this->desc = desc;
	}
};

struct FunctionElementException : public std::exception {
	std::string desc;
	const char * what() const throw ();
	FunctionElementException() {}
	FunctionElementException(std::string desc)
	{
		this->desc = desc;
	}
};



//struct InvalidReferenceIdException : public std::exception {
//	std::string desc;
//	const char * what() const throw ();
//	InvalidReferenceIdException() {}
//	InvalidReferenceIdException(std::string desc)
//	{
//		this->desc = desc;
//	}
//};



#endif
