#ifndef _BASE_NODE_DESCRIPTOR_TEST_HPP_
#define _BASE_NODE_DESCRIPTOR_TEST_HPP_

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include "JsonSerializer/JsonSerializer.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *NODE_DESCRIPTORS_FILE = "BaseNodeDescriptors.json";

class BaseNodeDescriptorTests : public ::testing::Test,
                                public ::testing::WithParamInterface<BaseNodeDescription>
{
};

TEST_P(BaseNodeDescriptorTests, isNodeClassEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>(NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().nodeClass, nodeDescriptor.nodeClass);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>((NODE_DESCRIPTORS_FILE, test_name);

  EXPECT_EQ(GetParam().writableFlag, nodeDescriptor.writableFlag);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>((NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().uniqueId, nodeDescriptor.uniqueId);
}

TEST_P(BaseNodeDescriptorTests, isLocaleEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>((NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().locale, nodeDescriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>((NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().displayName, nodeDescriptor.displayName);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription(NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().browseName, nodeDescriptor.browseName);
}

TEST_P(BaseNodeDescriptorTests, isDescriptionEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getNodeDescription<BaseNodeDescription>((NODE_DESCRIPTORS_FILE, test_name);

  ASSERT_STREQ(GetParam().description, nodeDescriptor.description);
}

std::vector<BaseNodeDescription> getBaseNodeDescriptors(const char *baseNodeDescriptorsJsonFile)
{
  std::vector<BaseNodeDescription> baseNodeDescriptors = setUpNodeDescriptors<BaseNodeDescription>(baseNodeDescriptorsJsonFile);
  return baseNodeDescriptors;
}

INSTANTIATE_TEST_SUITE_P(ParametrizedBaseNodeDescriptorTests, BaseNodeDescriptorTests,
                         ValuesIn(getBaseNodeDescriptors("BaseNodeDescriptors.json")));

#endif //_BASE_NODE_DESCRIPTOR_TEST_HPP_