#ifndef _NODE_DESCRIPTORS_TEST_UTILS_HPP
#define _NODE_DESCRIPTORS_TEST_UTILS_HPP

#include "JsonSerializer/JsonSerializer.hpp"

template <typename T>
T getNodeDescription(const char *nodeDescriptorName, std::string test_name)
{
    int test_number = std::stoi(test_name.substr(test_name.find("/") + 1));

    T nodeDescriptor = setUpNodeDescriptors<T>(nodeDescriptorName)[test_number];

    return nodeDescriptor;
}

template <typename T>
std::vector<T> getNodeDescriptors(const char *nodeDescriptorsJsonFile)
{
    std::vector<T> nodeDescriptors = setUpNodeDescriptors<T>(nodeDescriptorsJsonFile);
    return nodeDescriptors;
}

#endif //_NODE_DESCRIPTORS_TEST_UTILS_HPP