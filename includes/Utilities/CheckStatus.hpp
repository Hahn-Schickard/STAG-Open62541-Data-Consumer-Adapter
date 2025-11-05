#ifndef __OPEN62541_UTILITY_CHECK_STATUS_HPP
#define __OPEN62541_UTILITY_CHECK_STATUS_HPP

#include <open62541/types.h>

#include <stdexcept>
#include <string>

namespace open62541 {
struct StatusCodeNotGood : public std::runtime_error {
  StatusCodeNotGood(const std::string& msg, const UA_StatusCode& code);

  UA_StatusCode status() const;

private:
  UA_StatusCode status_;
};

void checkStatusCode(const std::string& msg, const UA_StatusCode& status,
    bool uncertain_is_bad = false);

void checkStatusCode(
    const UA_StatusCode& status, bool uncertain_is_bad = false);
} // namespace open62541

#endif //__OPEN62541_UTILITY_CHECK_STATUS_HPP