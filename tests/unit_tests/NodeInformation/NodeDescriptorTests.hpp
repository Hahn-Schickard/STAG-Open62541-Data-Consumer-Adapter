#ifndef _NODE_DESCRIPTOR_TESTS_HPP_
#define _NODE_DESCRIPTOR_TESTS_HPP_

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *NODE_DESCRIPTORS_FILE = "test_inputs/NodeDescriptors.json";
const char *PASSING_NODE_DESCRIPTORS_FILE = "expected_results/PassingNodeDescriptors.json";
const char *FAILLING_NODE_DESCRIPTORS_FILE = "expected_results/FaillingNodeDescriptors.json";

class NodeDescriptorTests : public ::testing::Test,
                            public ::testing::WithParamInterface<NodeDescription>
{
};

TEST_P(NodeDescriptorTests, isBaseNodeDescriptorEqual)
{
    std::string test_name = getCurrentTestName();

    NodeDescription node_descriptor = getNodeDescription<NodeDescription>(PASSING_NODE_DESCRIPTORS_FILE, test_name);
    NodeDescription expected_data = GetParam();

    EXPECT_STREQ(expected_data.base_node_descriptor.browse_name, node_descriptor.base_node_descriptor.browse_name)
        << "Browse names are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.browse_name << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.browse_name << std::endl;
    EXPECT_STREQ(expected_data.base_node_descriptor.description, node_descriptor.base_node_descriptor.description)
        << "Descriptions are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.description << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.description << std::endl;
    EXPECT_STREQ(expected_data.base_node_descriptor.display_name, node_descriptor.base_node_descriptor.display_name)
        << "Display names are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.display_name << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.display_name << std::endl;
    EXPECT_STREQ(expected_data.base_node_descriptor.locale, node_descriptor.base_node_descriptor.locale)
        << "Locales are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.locale << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.locale << std::endl;
    EXPECT_EQ(expected_data.base_node_descriptor.node_class, node_descriptor.base_node_descriptor.node_class)
        << "Node class types are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.node_class << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.node_class << std::endl;
    EXPECT_STREQ(expected_data.base_node_descriptor.unique_id, node_descriptor.base_node_descriptor.unique_id)
        << "Unique ids are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.unique_id << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.unique_id << std::endl;
    EXPECT_EQ(expected_data.base_node_descriptor.writable_flag, node_descriptor.base_node_descriptor.writable_flag)
        << "Writable flags are not equal for test:" << test_name << std::endl
        << "expected: " << expected_data.base_node_descriptor.writable_flag << std::endl
        << "provided: " << node_descriptor.base_node_descriptor.writable_flag << std::endl;
}

INSTANTIATE_TEST_SUITE_P(ParametrizedNodeDescriptorTests, NodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<NodeDescription>(NODE_DESCRIPTORS_FILE)));

#endif //_NODE_DESCRIPTOR_TESTS_HPP_