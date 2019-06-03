#include "NodeInformation.h"
#include "JsonSerializer/JsonSerializer.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

#define MAX_BASE_NODE_DESCRITPOR_TEST_COUNT 3
#define MAX_VARIABLE_NODE_DESCRIPTOR_TEST_COUNT 1
#define BASE_NODE_DESCRIPTOR_FIELD_COUNT 7
#define VARIABLE_NODE_DESCRIPTOR_FIELD_COUNT 2

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

template <typename T>
std::vector<T> setUpNodeDescriptors(const char *inputFile)
{
  std::ifstream jsonFile;

  jsonFile.open(inputFile, std::ifstream::in);

  if (!jsonFile)
  {
    std::cerr << "Json file not found!" << std::endl;
    exit(1);
  }

  std::vector<T> nodeDescriptors = setUpNodeDescriptor<T>(jsonFile);
  jsonFile.close();

  return nodeDescriptors;
}

BaseNodeDescription getBaseNodeDescription(std::string test_name)
{
  int test_number = std::stoi(test_name.substr(test_name.find("/") + 1));

  BaseNodeDescription nodeDescriptor = setUpNodeDescriptors<BaseNodeDescription>("BaseNodeDescriptors.json")[test_number];

  return nodeDescriptor;
}

class BaseNodeDescriptorTests : public ::testing::Test,
                                public ::testing::WithParamInterface<BaseNodeDescription>
{
};

TEST_P(BaseNodeDescriptorTests, isNodeClassEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  EXPECT_EQ(GetParam().nodeClass, nodeDescriptor.nodeClass);
}

TEST_P(BaseNodeDescriptorTests, isWritableFlagEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  EXPECT_EQ(GetParam().writableFlag, nodeDescriptor.writableFlag);
}

TEST_P(BaseNodeDescriptorTests, isUniqueIdEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  ASSERT_STREQ(GetParam().uniqueId, nodeDescriptor.uniqueId);
}

TEST_P(BaseNodeDescriptorTests, isLocaleEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  ASSERT_STREQ(GetParam().locale, nodeDescriptor.locale);
}

TEST_P(BaseNodeDescriptorTests, isDisplayNameEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  ASSERT_STREQ(GetParam().displayName, nodeDescriptor.displayName);
}

TEST_P(BaseNodeDescriptorTests, isBrowseNameEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  ASSERT_STREQ(GetParam().browseName, nodeDescriptor.browseName);
}

TEST_P(BaseNodeDescriptorTests, isDescriptionEqual)
{
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());

  BaseNodeDescription nodeDescriptor = getBaseNodeDescription(test_name);

  ASSERT_STREQ(GetParam().description, nodeDescriptor.description);
}

std::vector<BaseNodeDescription> getBaseNodeDescriptors(const char *baseNodeDescriptorsJsonFile)
{
  std::vector<BaseNodeDescription> baseNodeDescriptors = setUpNodeDescriptors<BaseNodeDescription>(baseNodeDescriptorsJsonFile);
  return baseNodeDescriptors;
}

INSTANTIATE_TEST_SUITE_P(ParametrizedBaseNodeDescriptorTests, BaseNodeDescriptorTests,
                         ValuesIn(getBaseNodeDescriptors("BaseNodeDescriptors.json")));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
