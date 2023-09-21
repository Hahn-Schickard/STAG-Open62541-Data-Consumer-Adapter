#ifndef __DCA_OPEN62541_CONFIG_SERIALIZER_HPP
#define __DCA_OPEN62541_CONFIG_SERIALIZER_HPP

#include "Config.hpp"

#include <string>

namespace open62541 {
Config deserializeConfig(const std::string& file_path);
void serializeConfig(const std::string& file_path, const Config& config);
void dumpDefaultConfig();
} // namespace open62541

#endif //__DCA_OPEN62541_CONFIG_SERIALIZER_HPP