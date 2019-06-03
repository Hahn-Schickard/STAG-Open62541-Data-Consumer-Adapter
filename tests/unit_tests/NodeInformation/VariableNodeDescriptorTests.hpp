#ifndef _VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_
#define _VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *VARIABLE_NODE_DESCRIPTORS_FILE = "VariableNodeDescriptors.json";

class VariableNodeDescriptorTests : public ::testing::Test,
                                    public ::testing::WithParamInterface<VariableNodeDescription>
{
};

TEST_P(VariableNodeDescriptorTests, isDataTypeEqual)
{
    std::string test_name = getCurrentTestName();

    VariableNodeDescription nodeDescriptor = getNodeDescription<VariableNodeDescription>(VARIABLE_NODE_DESCRIPTORS_FILE, test_name);

    EXPECT_EQ(GetParam().dataType, nodeDescriptor.dataType);
}

INSTANTIATE_TEST_SUITE_P(ParametrizedVariableNodeDescriptorTests, VariableNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<VariableNodeDescription>(VARIABLE_NODE_DESCRIPTORS_FILE)));

#endif //_VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_