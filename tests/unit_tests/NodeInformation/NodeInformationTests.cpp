#include "NodeInformation.h"
#include "JsonDeserializer.hpp"
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

const char
    *baseNodeDescriptorInformation[MAX_BASE_NODE_DESCRITPOR_TEST_COUNT]
                                  [BASE_NODE_DESCRIPTOR_FIELD_COUNT] = {
                                      {"UNRECOGNIZED_NODE", "false", "en-US",
                                       "1", "Base Node Descriptor Test 1",
                                       "base_node_descriptor_test_1",
                                       "This is some sort of a description"},
                                      {"UNRECOGNIZED_NODE", "true", "en-US",
                                       "2", "Base Node Descriptor Test 2",
                                       "base_node_descriptor_test_2",
                                       "This is some sort of a description"},
                                      {"VARIABLE_NODE", "false", "en-US", "3",
                                       "Base Node Descriptor Test 3",
                                       "base_node_descriptor_test_3",
                                       "This is some sort of a description"}};

const char
    *variableNodeDescriptorInformation[MAX_VARIABLE_NODE_DESCRIPTOR_TEST_COUNT]
                                      [VARIABLE_NODE_DESCRIPTOR_FIELD_COUNT] = {
                                          {"Boolean", "true"}};

bool identifyBoolean(const char *booleanValue)
{
  char *processedValue = convertToUpperCase(booleanValue);
  bool booleanFlag =
      (strcmp("TRUE", processedValue) || strcmp("Y", processedValue)) ? true
                                                                      : false;
  free(processedValue);
  return booleanFlag;
}

NodeClassType identifyNodeClassType(const char *nodeClassTypeString)
{
  char *processedValue = convertToUpperCase(nodeClassTypeString);
  switch (hash(processedValue))
  {
  case hash("VARIABLE_NODE"):
  {
    free(processedValue);
    return NodeClassType::VARIABLE_NODE;
  }
  case hash("METHOD_NODE"):
  {
    free(processedValue);
    return NodeClassType::METHOD_NODE;
  }
  case hash("OBJECT_NODE"):
  {
    free(processedValue);
    return NodeClassType::OBJECT_NODE;
  }
  case hash("UNRECOGNIZED_NODE"):
  default:
  {
    free(processedValue);
    return NodeClassType::UNRECOGNIZED_NODE;
  }
  }
}

DataType identifyDataType(const char *dataTypeString)
{
  char *processedValue = convertToUpperCase(dataTypeString);
  switch (hash(processedValue))
  {
  case hash("UNSIGNEDSHORT"):
  {
    free(processedValue);
    return DataType::UnsignedShort;
  }
  case hash("UNSIGNEDINTEGER"):
  {
    free(processedValue);
    return DataType::UnsignedInteger;
  }
  case hash("UNSIGNEDLONG"):
  {
    free(processedValue);
    return DataType::UnsignedLong;
  }
  case hash("SIGNEDSHORT"):
  {
    free(processedValue);
    return DataType::SignedShort;
  }
  case hash("SIGNEDINTEGER"):
  {
    free(processedValue);
    return DataType::SignedInteger;
  }
  case hash("SIGNEDLONG"):
  {
    free(processedValue);
    return DataType::SignedLong;
  }
  case hash("DOUBLE"):
  {
    free(processedValue);
    return DataType::Double;
  }
  case hash("BOOLEAN"):
  {
    free(processedValue);
    return DataType::Boolean;
  }
  case hash("STRING"):
  {
    free(processedValue);
    return DataType::String;
  }
  case hash("UNKNOWN"):
  default:
  {
    free(processedValue);
    return DataType::Unknown;
  }
  }
}

DataValue identifyDataValue(DataType dataType, const char *dataValueString)
{
  DataValue dataValue;

  switch (dataType)
  {
  case DataType::UnsignedShort:
  {
    dataValue.unsigned_short_value = (uint8_t)atoi(dataValueString);
    break;
  }
  case DataType::UnsignedInteger:
  {
    dataValue.unsigned_integer_value = (uint32_t)atoi(dataValueString);
    break;
  }
  case DataType::UnsignedLong:
  {
    dataValue.unsigned_long_value = (uint64_t)atoi(dataValueString);
    break;
  }
  case DataType::SignedShort:
  {
    dataValue.signed_short_value = (uint8_t)atoi(dataValueString);
    break;
  }
  case DataType::SignedInteger:
  {
    dataValue.signed_integer_value = (int32_t)atoi(dataValueString);
    break;
  }
  case DataType::SignedLong:
  {
    dataValue.signed_long_value = (int64_t)atoi(dataValueString);
    break;
  }
  case DataType::Double:
  {
    dataValue.double_value = atof(dataValueString);
    break;
  }
  case DataType::Boolean:
  {
    dataValue.boolean_value = identifyBoolean(dataValueString);
    break;
  }
  case DataType::String:
  {
    copyCharArray(dataValueString, dataValue.string_value);
    break;
  }
  case DataType::Unknown:
  {
    break;
  }
  }
}

BaseNodeDescription
setUpBaseNodeDescriptor(const char *nodeClassType, const char *writableFlag,
                        const char *locale, const char *uniqueId,
                        const char *displayName, const char *browseName,
                        const char *description)
{
  BaseNodeDescription baseNodeDescritpor;
  baseNodeDescritpor.nodeClass = identifyNodeClassType(nodeClassType);
  baseNodeDescritpor.writableFlag = identifyBoolean(writableFlag);
  copyCharArray(locale, baseNodeDescritpor.locale);
  copyCharArray(uniqueId, baseNodeDescritpor.uniqueId);
  copyCharArray(displayName, baseNodeDescritpor.displayName);
  copyCharArray(browseName, baseNodeDescritpor.browseName);
  copyCharArray(description, baseNodeDescritpor.description);

  return baseNodeDescritpor;
}

VariableNodeDescription setUpVariableNodeDescriptor(const char *dataType,
                                                    const char *dataValue)
{
  VariableNodeDescription nodeDescriptor = {
      .dataType = identifyDataType(dataType),
      .dataValue = identifyDataValue(identifyDataType(dataType), dataValue)};
  return nodeDescriptor;
}

// class BaseNodeDescriptorTest : public TestWithParam<const char *> {
// protected:
//   BaseNodeDescription baseNodeDescriptor;

// public:
//   void SetUp() override {
//     baseNodeDescriptor = setUpBaseNodeDescriptor(
//         (const char *)GetParam(), (const char *)GetParam(),
//         (const char *)GetParam(), (const char *)GetParam(),
//         (const char *)GetParam(), (const char *)GetParam(),
//         (const char *)GetParam());
//   }
// };

TEST(NodeInformationTests, BaseNodeDescriptorContentFormatTest)
{
  for (int i = 0; i < MAX_BASE_NODE_DESCRITPOR_TEST_COUNT; i++)
  {
    BaseNodeDescription baseNodeDescriptor =
        setUpBaseNodeDescriptor(baseNodeDescriptorInformation[i][0],
                                baseNodeDescriptorInformation[i][1],
                                baseNodeDescriptorInformation[i][2],
                                baseNodeDescriptorInformation[i][3],
                                baseNodeDescriptorInformation[i][4],
                                baseNodeDescriptorInformation[i][5],
                                baseNodeDescriptorInformation[i][6]);

    EXPECT_EQ(identifyNodeClassType(baseNodeDescriptorInformation[i][0]),
              baseNodeDescriptor.nodeClass);
    EXPECT_EQ(identifyBoolean(baseNodeDescriptorInformation[i][1]),
              baseNodeDescriptor.writableFlag);

    EXPECT_STREQ(baseNodeDescriptorInformation[i][2],
                 baseNodeDescriptor.locale);
    EXPECT_STREQ(baseNodeDescriptorInformation[i][3],
                 baseNodeDescriptor.uniqueId);
    EXPECT_STREQ(baseNodeDescriptorInformation[i][4],
                 baseNodeDescriptor.displayName);
    EXPECT_STREQ(baseNodeDescriptorInformation[i][5],
                 baseNodeDescriptor.browseName);
    EXPECT_STREQ(baseNodeDescriptorInformation[i][6],
                 baseNodeDescriptor.description);
  }
}

TEST(NodeInformationTests, BaseNodeDescriptorJsonFileTEst)
{
  std::ifstream jsonFile;
  jsonFile.open("BaseNodeDescriptors.json", std::ifstream::in);

  if (!jsonFile)
  {
    // Print an error and exit
    std::cerr << "File not found!" << std::endl;
    exit(1);
  }

  std::vector<BaseNodeDescription> baseNodeDescriptors = setUpBaseNodeDescriptor(jsonFile);
  BaseNodeDescription baseNodeDescriptor1 = baseNodeDescriptors[0];
  BaseNodeDescription baseNodeDescriptor2 = baseNodeDescriptors[1];
  for (int i = 0; i < MAX_BASE_NODE_DESCRITPOR_TEST_COUNT; i++)
  {
    // EXPECT_EQ(identifyNodeClassType(baseNodeDescriptorInformation[i][0]),
    //           baseNodeDescriptor.nodeClass);
    // EXPECT_EQ(identifyBoolean(baseNodeDescriptorInformation[i][1]),
    //           baseNodeDescriptor.writableFlag);

    // EXPECT_STREQ(baseNodeDescriptorInformation[i][2],
    //              baseNodeDescriptor.locale);
    // EXPECT_STREQ(baseNodeDescriptorInformation[i][3],
    //              baseNodeDescriptor.uniqueId);
    // EXPECT_STREQ(baseNodeDescriptorInformation[i][4],
    //              baseNodeDescriptor.displayName);
    // EXPECT_STREQ(baseNodeDescriptorInformation[i][5],
    //              baseNodeDescriptor.browseName);
    // EXPECT_STREQ(baseNodeDescriptorInformation[i][6],
    //              baseNodeDescriptor.description);
  }
}

// TEST_P(BaseNodeDescriptorTest, ReturnsCorrectNodeClass) {
//   EXPECT_EQ(identifyNodeClassType((const char *)GetParam()),
//             baseNodeDescriptor.nodeClass);
// }

TEST(NodeInformationTests, VariableNodeDescriptorContetFormatTest)
{
  for (int i = 0; i < MAX_VARIABLE_NODE_DESCRIPTOR_TEST_COUNT; i++)
  {
    VariableNodeDescription variableNodeDescriptor =
        setUpVariableNodeDescriptor(variableNodeDescriptorInformation[i][0],
                                    variableNodeDescriptorInformation[i][1]);

    EXPECT_EQ(identifyDataType(variableNodeDescriptorInformation[i][0]),
              variableNodeDescriptor.dataType);
  }
}

TEST(NodeInformationTests, NodeDescriptorContetFormatTest) {}

// std::ifstream inputValuesFromFile("input.txt");
// INSTANTIATE_TEST_CASE_P(
//     FromFileStream, BaseNodeDescriptorTest,
//     ValuesIn(std::istream_iterator<const char *>(inputValuesFromFile),
//              std::istream_iterator<const char *>()));

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}