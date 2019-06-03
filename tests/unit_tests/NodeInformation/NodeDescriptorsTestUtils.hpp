#ifndef _NODE_DESCRIPTORS_TEST_UTILS_HPP
#define _NODE_DESCRIPTORS_TEST_UTILS_HPP

template <typename T>
T getNodeDescription(const char *nodeDescriptorName, std::string test_name)
{
    int test_number = std::stoi(test_name.substr(test_name.find("/") + 1));

    T nodeDescriptor = setUpNodeDescriptors<T>()[test_number];

    return nodeDescriptor;
}

#endif //_NODE_DESCRIPTORS_TEST_UTILS_HPP