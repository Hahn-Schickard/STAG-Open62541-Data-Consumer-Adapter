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

  EXPECT_EQ(GetParam().node_class, node_descriptor.node_class);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().writable_flag, node_descriptor.writable_flag);
}

TEST_P(BaseNodeDescriptorTests, isLocaleEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().locale, node_descriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().unique_id, node_descriptor.unique_id);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().display_name, node_descriptor.display_name);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().browse_name, node_descriptor.browse_name);
}

//------------------------------------------------------------------------------------------------------------------------------

TEST_P(BaseNodeDescriptorTests, isNodeClassNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_NE(GetParam().node_class, node_descriptor.node_class);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_NE(GetParam().writable_flag, node_descriptor.writable_flag);
}

TEST_P(BaseNodeDescriptorTests, isLocaleNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().locale, node_descriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().unique_id, node_descriptor.unique_id);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().display_name, node_descriptor.display_name);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().browse_name, node_descriptor.browse_name);
}

TEST_P(BaseNodeDescriptorTests, isDescriptionNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription node_descriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().description, node_descriptor.description);
}

INSTANTIATE_TEST_SUITE_P(ParametrizedBaseNodeDescriptorTests, BaseNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE)));

#endif //_BASE_NODE_DESCRIPTOR_TEST_HPP_