#include "CheckStatus.hpp"

namespace open62541 {
using namespace std;

StatusCodeNotGood::StatusCodeNotGood(
    const string& msg, const UA_StatusCode& code)
    : runtime_error("Received status code: " +
          string(UA_StatusCode_name(code)) + (msg.empty() ? "" : " " + msg)),
      status_(code) {}

UA_StatusCode StatusCodeNotGood::status() const { return status_; }

void checkStatusCode(
    const string& msg, const UA_StatusCode& status, bool uncertain_is_bad) {
  if (UA_StatusCode_isBad(status) ||
      (UA_StatusCode_isUncertain(status) && uncertain_is_bad)) {
    throw StatusCodeNotGood(msg, status);
  }
}

void checkStatusCode(const UA_StatusCode& status, bool uncertain_is_bad) {
  checkStatusCode(string(), status, uncertain_is_bad);
}
} // namespace open62541