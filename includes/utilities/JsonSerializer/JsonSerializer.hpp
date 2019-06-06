#ifndef _JSON_DESERIALIZER_HPP
#define _JSON_DESERIALIZER_HPP

#include "JsonSerializerHeader.hpp"
#include "NodeDescriptionSerializers/BaseNodeDescriptionSerializer.hpp"
#include "NodeDescriptionSerializers/VariableNodeDescriptionSerializer.hpp"
#include "NodeDescriptionSerializers/ObjectNodeDescriptionSerializer.hpp"
#include "NodeDescriptionSerializers/NodeDescriptionSerializer.hpp"

#include <stdio.h>  // defines FILENAME_MAX
#include <unistd.h> // for getcwd()

namespace nlohmann
{
template <typename T>
std::vector<T> setUpNodeDescriptor(std::ifstream &file_stream)
{

    json json_file;
    file_stream >> json_file;
    std::vector<T> node_descriptors = json_file;
    return node_descriptors;
}

template <typename T>
std::vector<T> setUpNodeDescriptors(const char *input_file)
{
    std::ifstream json_file;

    json_file.open(input_file, std::ifstream::in);

    if (!json_file)
    {
        std::string cwd("\0", FILENAME_MAX + 1);
        std::cerr << "Json file: " << input_file << " not found!" << std::endl
                  << "I am running at: " << getcwd(&cwd[0], cwd.capacity())
                  << std::endl;
        exit(1);
    }

    std::vector<T> node_descriptors = setUpNodeDescriptor<T>(json_file);
    json_file.close();

    return node_descriptors;
}

template <typename T>
T deserializeNodeDescriptor(nlohmann::json &json_descriptor)
{
    return json_descriptor.get<T>();
}

template std::vector<BaseNodeDescription> setUpNodeDescriptor(std::ifstream &file_stream);
template std::vector<VariableNodeDescription> setUpNodeDescriptor(std::ifstream &file_stream);
template std::vector<ObjectNodeDescription> setUpNodeDescriptor(std::ifstream &file_stream);
//template std::vector<MethodNodeDescription> setUpNodeDescriptor(std::ifstream &file_stream);
template std::vector<NodeDescription> setUpNodeDescriptor(std::ifstream &file_stream);
} // namespace nlohmann
#endif //_JSON_DESERIALIZER_HPP