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

INSTANTIATE_TEST_SUITE_P(ParametrizedNodeDescriptorTests, NodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<NodeDescription>(NODE_DESCRIPTORS_FILE)));

#endif //_NODE_DESCRIPTOR_TESTS_HPP_