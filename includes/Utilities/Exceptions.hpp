#ifndef __OPEN62541_UTILITY_EXCEPTIONS_HPP
#define __OPEN62541_UTILITY_EXCEPTIONS_HPP

#include <stdexcept>

namespace open62541 {
struct OutOfMemory : std::runtime_error {
  OutOfMemory()
      : std::runtime_error(
            "There is not enough memory to complete the operation") {}
};

} // namespace open62541
#endif //__OPEN62541_UTILITY_EXCEPTIONS_HPP