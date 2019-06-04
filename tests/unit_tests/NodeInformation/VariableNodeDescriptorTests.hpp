#ifndef _VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_
#define _VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *VARIABLE_NODE_DESCRIPTORS_FILE = "test_inputs/VariableNodeDescriptors.json";
const char *PASSING_VARIABLE_NODE_DESCRIPTORS_FILE = "expected_results/PassingVariableNodeDescriptors.json";
const char *FAILLING_VARIABLE_NODE_DESCRIPTORS_FILE = "expected_results/FaillingVariableNodeDescriptors.json";

class VariableNodeDescriptorTests : public ::testing::Test,
                                    public ::testing::WithParamInterface<VariableNodeDescription>
{
};

TEST_P(VariableNodeDescriptorTests, isDataTypeEqual)
{
    std::string test_name = getCurrentTestName();

    VariableNodeDescription nodeDescriptor = getNodeDescription<VariableNodeDescription>(PASSING_VARIABLE_NODE_DESCRIPTORS_FILE, test_name);

    EXPECT_EQ(GetParam().dataType, nodeDescriptor.dataType);
}

TEST_P(VariableNodeDescriptorTests, isDataTypeNotEqual)
{
    std::string test_name = getCurrentTestName();

    VariableNodeDescription nodeDescriptor = getNodeDescription<VariableNodeDescription>(FAILLING_VARIABLE_NODE_DESCRIPTORS_FILE, test_name);

    EXPECT_NE(GetParam().dataType, nodeDescriptor.dataType);
}

INSTANTIATE_TEST_SUITE_P(ParametrizedVariableNodeDescriptorTests, VariableNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<VariableNodeDescription>(VARIABLE_NODE_DESCRIPTORS_FILE)));

#endif //_VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_