#ifndef _BASE_NODE_DESCRIPTOR_TEST_HPP_
#define _BASE_NODE_DESCRIPTOR_TEST_HPP_

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *BASE_NODE_DESCRIPTORS_FILE = "BaseNodeDescriptors.json";

class BaseNodeDescriptorTests : public ::testing::Test,
                                public ::testing::WithParamInterface<BaseNodeDescription>
{
};

TEST_P(BaseNodeDescriptorTests, isNodeClassEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().nodeClass, nodeDescriptor.nodeClass);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().writableFlag, nodeDescriptor.writableFlag);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().uniqueId, nodeDescriptor.uniqueId);
}

TEST_P(BaseNodeDescriptorTests, isLocaleEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().locale, nodeDescriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().displayName, nodeDescriptor.displayName);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameEqual)
{
  std::string test_name = getCurrentTestName();

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().browseName, nodeDescriptor.browseName);
}

TEST_P(BaseNodeDescriptorTests, isDescriptionEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().description, nodeDescriptor.description);
}

INSTANTIATE_TEST_SUITE_P(ParametrizedBaseNodeDescriptorTests, BaseNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<BaseNodeDescription>(BASE_NODE_DESCRIPTORS_FILE)));

#endif //_BASE_NODE_DESCRIPTOR_TEST_HPP_