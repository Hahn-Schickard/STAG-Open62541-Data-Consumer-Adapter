#ifndef _BASE_NODE_DESCRIPTOR_TEST_HPP_
#define _BASE_NODE_DESCRIPTOR_TEST_HPP_

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *BASE_NODE_DESCRIPTORS_FILE = "test_inputs/BaseNodeDescriptors.json";
const char *PASSING_BASE_NODE_DESCRIPTORS_FILE = "expected_results/PassingBaseNodeDescriptors.json";
const char *FAILLING_BASE_NODE_DESCRIPTORS_FILE = "expected_results/FaillingBaseNodeDescriptors.json";

class BaseNodeDescriptorTests : public ::testing::Test,
                                public ::testing::WithParamInterface<BaseNodeDescription>
{
};

TEST_P(BaseNodeDescriptorTests, isNodeClassEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  EXPECT_EQ(expected_data.node_class, node_descriptor.node_class)
      << "Node class types are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.node_class << std::endl
      << "provided: " << node_descriptor.node_class << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  EXPECT_EQ(expected_data.writable_flag, node_descriptor.writable_flag)
      << "Writable flags are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.writable_flag << std::endl
      << "provided: " << node_descriptor.writable_flag << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isLocaleEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STREQ(expected_data.locale, node_descriptor.locale)
      << "Locales are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.locale << std::endl
      << "provided: " << node_descriptor.locale << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STREQ(expected_data.unique_id, node_descriptor.unique_id)
      << "Unique ids are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.unique_id << std::endl
      << "provided: " << node_descriptor.unique_id << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STREQ(expected_data.display_name, node_descriptor.display_name)
      << "Display names are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.display_name << std::endl
      << "provided: " << node_descriptor.display_name << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STREQ(expected_data.browse_name, node_descriptor.browse_name)
      << "Browse names are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.browse_name << std::endl
      << "provided: " << node_descriptor.browse_name << std::endl;
}

//------------------------------------------------------------------------------------------------------------------------------

TEST_P(BaseNodeDescriptorTests, isNodeClassNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  EXPECT_NE(expected_data.node_class, node_descriptor.node_class)
      << "Node class types are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.node_class << std::endl
      << "provided: " << node_descriptor.node_class << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  EXPECT_NE(expected_data.writable_flag, node_descriptor.writable_flag)
      << "Writable flags are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.writable_flag << std::endl
      << "provided: " << node_descriptor.writable_flag << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isLocaleNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STRNE(expected_data.locale, node_descriptor.locale)
      << "Locales are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.locale << std::endl
      << "provided: " << node_descriptor.locale << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STRNE(expected_data.unique_id, node_descriptor.unique_id)
      << "Unique ids are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.unique_id << std::endl
      << "provided: " << node_descriptor.unique_id << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STRNE(expected_data.display_name, node_descriptor.display_name)
      << "Display names are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.display_name << std::endl
      << "provided: " << node_descriptor.display_name << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STRNE(expected_data.browse_name, node_descriptor.browse_name)
      << "Browse names are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.browse_name << std::endl
      << "provided: " << node_descriptor.browse_name << std::endl;
}

TEST_P(BaseNodeDescriptorTests, isDescriptionNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);
  BaseNodeDescription expected_data = GetParam();

  ASSERT_STRNE(expected_data.description, node_descriptor.description)
      << "Descriptions are equal for test:" << test_name << std::endl
      << "expected: " << expected_data.description << std::endl
      << "provided: " << node_descriptor.description << std::endl;
}

INSTANTIATE_TEST_SUITE_P(ParametrizedBaseNodeDescriptorTests, BaseNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE)));

#endif //_BASE_NODE_DESCRIPTOR_TEST_HPP_