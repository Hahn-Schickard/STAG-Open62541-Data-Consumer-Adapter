#ifndef __DCA_OPEN62541_CONFIG_SERIALZER_HPP
#define __DCA_OPEN62541_CONFIG_SERIALZER_HPP

#include "Config.hpp"

#include <string>

namespace open62541 {
const Config deserializeConfig(const std::string &file_path);
void serializeConfig(const std::string &file_path, const Config &config);
} // namespace open62541

#endif //__DCA_OPEN62541_CONFIG_SERIALZER_HPP