#ifndef _JSON_DESERIALIZER_HPP
#define _JSON_DESERIALIZER_HPP

#include "JsonSerializerHeader.hpp"
#include "NodeDescriptionSerializers/BaseNodeDescriptionSerializer.hpp"
#include "NodeDescriptionSerializers/VariableNodeDescriptionSerializer.hpp"

using json = nlohmann::json;

template <typename T>
std::vector<T> setUpNodeDescriptor(std::ifstream &fileStream)
{

    json jsonFile;
    fileStream >> jsonFile;
    std::vector<T> nodeDescriptors = jsonFile;
    return nodeDescriptors;
}

template <typename T>
T deserializeNodeDescriptor(nlohmann::json &jsonDescriptor)
{
    return jsonDescriptor.get<T>();
}

template std::vector<BaseNodeDescription> setUpNodeDescriptor(std::ifstream &fileStream);
template std::vector<VariableNodeDescription> setUpNodeDescriptor(std::ifstream &fileStream);

#endif //_JSON_DESERIALIZER_HPP