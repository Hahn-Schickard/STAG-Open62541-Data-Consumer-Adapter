#ifndef _VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_
#define _VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_

#include "NodeDescriptorsTestUtils.hpp"
#include "NodeInformation.h"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *VARIABLE_NODE_DESCRIPTORS_FILE =
    "test_inputs/VariableNodeDescriptors.json";
const char *PASSING_VARIABLE_NODE_DESCRIPTORS_FILE =
    "expected_results/VariableNodeResults/PassingVariableNodeDescriptors.json";
const char *FAILLING_VARIABLE_NODE_DESCRIPTORS_FILE =
    "expected_results/VariableNodeResults/FaillingVariableNodeDescriptors.json";

class VariableNodeDescriptorTests
    : public ::testing::Test,
      public ::testing::WithParamInterface<VariableNodeDescription> {};

TEST_P(VariableNodeDescriptorTests, isDataTypeEqual) {
  std::string test_name = getCurrentTestName();

  VariableNodeDescription node_descriptor =
      getNodeDescription<VariableNodeDescription>(
          PASSING_VARIABLE_NODE_DESCRIPTORS_FILE, test_name);
  VariableNodeDescription expected_data = GetParam();

  EXPECT_EQ(expected_data.data.type, node_descriptor.data.type)
      << "Data type fields are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.data.type << std::endl
      << "provided: " << node_descriptor.data.type << std::endl;
}

TEST_P(VariableNodeDescriptorTests, isDataTypeNotEqual) {
  std::string test_name = getCurrentTestName();

  VariableNodeDescription node_descriptor =
      getNodeDescription<VariableNodeDescription>(
          FAILLING_VARIABLE_NODE_DESCRIPTORS_FILE, test_name);
  VariableNodeDescription expected_data = GetParam();

  EXPECT_NE(expected_data.data.type, node_descriptor.data.type)
      << "Data type fields are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.data.type << std::endl
      << "provided: " << node_descriptor.data.type << std::endl;
}

INSTANTIATE_TEST_SUITE_P(ParametrizedVariableNodeDescriptorTests,
                         VariableNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<VariableNodeDescription>(
                             VARIABLE_NODE_DESCRIPTORS_FILE)));

#endif //_VARIABLE_NODE_DESCRIPTOR_TESTS_HPP_