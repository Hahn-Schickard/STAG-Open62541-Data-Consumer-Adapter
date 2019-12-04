#ifndef _I_DEVICE_TYPE_HPP
#define _I_DEVICE_TYPE_HPP

#include <cstdlib>
#include <cxxabi.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace Information_Model {

  /**
    * @brief A generic data type wrapper for various fundemntal types and data containers
    * 
    * @tparam T 
    * 
    * @author Dovydas Girdvainis
    * @date 18.07.2019
    */
  template<class T>
  class DataWrapper {
   private:
    T value_;

   public:
    /**
    * @brief Constructs a new Data Wrapper object
    * 
    * @throw std::runtime_error if no specialization for the given type exists
    * 
    * @param value 
    */
    DataWrapper(T value) {
      setValue(value);
    }

    /**
     * @brief Returns the contained value object 
     * 
     * @throw std::runtime_error if no specialization for the given type exists
     * 
     * @return T& 
     */
    T& getValue() {
      if(std::is_fundamental<T>::value) {
	return value_;
      } else {
	throwNoSpecializationImplemented();
      }
    }

   private:
    void setValue(T value) {
      if(std::is_fundamental<T>::value) {
	value_ = value;
      } else {
	throwNoSpecializationImplemented();
      }
    }

    void throwNoSpecializationImplemented() {
      throw std::runtime_error("DataWrapper<" + typeName()
                               + ">"
                                 " does not have a specialization for type: "
                               + typeName());
    }

    /**
     * @brief Demangles given type name and returns it as a readable string
     * 
     * 
     * Kudos to Bunkar & M. Dudley
     * @https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
     */
    std::string typeName() {
      int status;
      std::string tname = typeid(T).name();
      char* demangled_name
          = abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
      if(status == 0) {
	tname = demangled_name;
	std::free(demangled_name);
      }
      return tname;
    }
  };

  /**
   * @brief Specializes DataWrapper::DataWrapper<T> for string data types
   * 
   * @tparam  
   */
  template<>
  class DataWrapper<std::string> {
   private:
    std::string value_;

   public:
    DataWrapper(std::string value)
        : value_(value) {}

    std::string getValue() {
      return value_;
    }
  };

}   // namespace Information_Model
#endif   //_I_DEVICE_TYPE_HPP