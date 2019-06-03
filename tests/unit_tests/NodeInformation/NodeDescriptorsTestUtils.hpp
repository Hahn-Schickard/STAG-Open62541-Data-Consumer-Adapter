#ifndef _NODE_DESCRIPTORS_TEST_UTILS_HPP
#define _NODE_DESCRIPTORS_TEST_UTILS_HPP

#include "JsonSerializer/JsonSerializer.hpp"
#include <string>

template <typename T>
T getNodeDescription(const char *nodeDescriptorName, std::string test_name)
{
    int test_number = std::stoi(test_name.substr(test_name.find("/") + 1));

    T nodeDescriptor = setUpNodeDescriptors<T>(nodeDescriptorName)[test_number];

    return nodeDescriptor;
}

std::string getCurrentTestName()
{
    const ::testing::TestInfo *const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    std::string test_name = std::string(test_info->name());
    return test_name;
}

template <typename T>
std::vector<T> getNodeDescriptors(const char *nodeDescriptorsJsonFile)
{
    std::vector<T> nodeDescriptors = setUpNodeDescriptors<T>(nodeDescriptorsJsonFile);
    return nodeDescriptors;
}

#endif //_NODE_DESCRIPTORS_TEST_UTILS_HPP