#ifndef INFORMATIONMODELEXCEPTONS_H
#define INFORMATIONMODELEXCEPTONS_H
#include <string>

struct InvalidReferenceIdException : public std::exception {
  std::string desc;
  const char *what() const throw() {
    std::string str = "Invalid ReferenceId: ";
    return (str + desc).c_str();
  }
  InvalidReferenceIdException() { this->desc = "Invalid Reference Id"; }
  InvalidReferenceIdException(std::string desc) { this->desc = desc; }
};

struct ElementTypeMismatchException : public std::exception {
  std::string desc;
  const char *what() const throw() {
    std::string str = "Element Types are mismatched: ";
    return (str + desc).c_str();
  }
  ElementTypeMismatchException() {}
  ElementTypeMismatchException(std::string desc) { this->desc = desc; }
};

struct FunctionElementException : public std::exception {
  std::string desc;
  const char *what() const throw() {
    std::string str = "FunctionElement caused an exception: ";
    return (str + desc).c_str();
  }
  FunctionElementException() {}
  FunctionElementException(std::string desc) { this->desc = desc; }
};

struct UndefinedElementTypeException : public std::exception {
  std::string desc;
  const char *what() const throw() {
    std::string str = "Undefined ElementType: ";
    return (str + desc).c_str();
  }
  UndefinedElementTypeException() {}
  UndefinedElementTypeException(std::string desc) { this->desc = desc; }
};

struct GroupElementDoesNotExistException : public std::exception {
  std::string desc;
  const char *what() const throw() {
    std::string str = "Group element does not exist : ";
    return (str + desc).c_str();
  }
  GroupElementDoesNotExistException() {}
  GroupElementDoesNotExistException(std::string desc) { this->desc = desc; }
};

#endif
