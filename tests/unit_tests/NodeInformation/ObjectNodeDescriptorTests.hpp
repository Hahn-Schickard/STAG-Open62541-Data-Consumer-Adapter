#ifndef _OBJECT_NODE_DESCRIPTOR_TESTS_HPP
#define _OBJECT_NODE_DESCRIPTOR_TESTS_HPP

#include "NodeDescriptorsTestUtils.hpp"
#include "NodeInformation.h"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *OBJECT_NODE_DESCRIPTORS_FILE =
    "test_inputs/ObjectNodeDescriptors.json";
const char *PASSING_OBJECT_NODE_DESCRIPTORS_FILE =
    "expected_results/ObjectNodeResults/PassingObjectNodeDescriptors.json";
const char *FAILLING_OBJECT_NODE_DESCRIPTORS_FILE =
    "expected_results/ObjectNodeResults/FaillingObjectNodeDescriptors.json";

class ObjectNodeDescriptorTests
    : public ::testing::Test,
      public ::testing::WithParamInterface<ObjectNodeDescription> {};

TEST_P(ObjectNodeDescriptorTests, isChildrenCountEqual) {
  std::string test_name = getCurrentTestName();

  ObjectNodeDescription node_descriptor =
      getNodeDescription<ObjectNodeDescription>(
          PASSING_OBJECT_NODE_DESCRIPTORS_FILE, test_name);
  ObjectNodeDescription expected_data = GetParam();

  EXPECT_EQ(expected_data.children_count, node_descriptor.children_count)
      << "Children count fields are not equal for test:" << test_name
      << std::endl
      << "expected: " << expected_data.children_count << std::endl
      << "provided: " << node_descriptor.children_count << std::endl;
}

TEST_P(ObjectNodeDescriptorTests, isChildrenCountNotEqual) {
  std::string test_name = getCurrentTestName();

  ObjectNodeDescription node_descriptor =
      getNodeDescription<ObjectNodeDescription>(
          FAILLING_OBJECT_NODE_DESCRIPTORS_FILE, test_name);
  ObjectNodeDescription expected_data = GetParam();

  EXPECT_NE(expected_data.children_count, node_descriptor.children_count)
      << "Children count fields are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.children_count << std::endl
      << "provided: " << node_descriptor.children_count << std::endl;
}

INSTANTIATE_TEST_SUITE_P(ParametrizedObjectNodeDescriptorTests,
                         ObjectNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<ObjectNodeDescription>(
                             OBJECT_NODE_DESCRIPTORS_FILE)));

#endif //_OBJECT_NODE_DESCRIPTOR_TESTS_HPP