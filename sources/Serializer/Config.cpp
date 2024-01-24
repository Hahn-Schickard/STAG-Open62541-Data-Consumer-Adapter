#include "Config.hpp"

namespace open62541 {

Config::~Config() {
  /*
    For the time being, we take care only of those fields which are not used
    by `Configuration::Configuration(const string& filepath)`. In the long run,
    we should
    - cover all fields
    - use open62541-style copy when using them
  */
  UA_String_clear(&access_credentials.username);
  UA_String_clear(&access_credentials.password);
}

} // namespace open62541
