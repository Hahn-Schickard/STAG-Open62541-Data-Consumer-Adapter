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

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().nodeClass, nodeDescriptor.nodeClass);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().writableFlag, nodeDescriptor.writableFlag);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().uniqueId, nodeDescriptor.uniqueId);
}

TEST_P(BaseNodeDescriptorTests, isLocaleEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().locale, nodeDescriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().displayName, nodeDescriptor.displayName);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(PASSING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().browseName, nodeDescriptor.browseName);
}

//------------------------------------------------------------------------------------------------------------------------------

TEST_P(BaseNodeDescriptorTests, isNodeClassNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_NE(GetParam().nodeClass, nodeDescriptor.nodeClass);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_NE(GetParam().writableFlag, nodeDescriptor.writableFlag);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().uniqueId, nodeDescriptor.uniqueId);
}

TEST_P(BaseNodeDescriptorTests, isLocaleNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().locale, nodeDescriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().displayName, nodeDescriptor.displayName);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().browseName, nodeDescriptor.browseName);
}

TEST_P(BaseNodeDescriptorTests, isDescriptionNotEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(FAILLING_BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STRNE(GetParam().description, nodeDescriptor.description);
}

INSTANTIATE_TEST_SUITE_P(ParametrizedBaseNodeDescriptorTests, BaseNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE)));

#endif //_BASE_NODE_DESCRIPTOR_TEST_HPP_