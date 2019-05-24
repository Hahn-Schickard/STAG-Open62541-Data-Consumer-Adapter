#include "NodeInformation.h"
#include "CStringFormater.hpp"

#include <gtest/gtest.h>

#define MAX_BASE_NODE_DESCRITPOR_TEST_COUNT 3
#define MAX_VARIABLE_NODE_DESCRIPTOR_TEST_COUNT 1
#define BASE_NODE_DESCRIPTOR_FIELD_COUNT 7
#define VARIABLE_NODE_DESCRIPTOR_FIELD_COUNT 2

const char *baseNodeDescriptorInformation[MAX_BASE_NODE_DESCRITPOR_TEST_COUNT][BASE_NODE_DESCRIPTOR_FIELD_COUNT] = {
    {"UNRECOGNIZED_NODE", "false", "en-US", "1", "Base Node Descriptor Test 1", "base_node_descriptor_test_1", "This is some sort of a description"},
    {"UNRECOGNIZED_NODE", "true", "en-US", "2", "Base Node Descriptor Test 2", "base_node_descriptor_test_2", "This is some sort of a description"},
    {"VARIABLE_NODE", "false", "en-US", "3", "Base Node Descriptor Test 3", "base_node_descriptor_test_3", "This is some sort of a description"}};

const char *variableNodeDescriptorInformation[MAX_VARIABLE_NODE_DESCRIPTOR_TEST_COUNT][VARIABLE_NODE_DESCRIPTOR_FIELD_COUNT] = {
    {"Boolean", "true"}};

bool identifyBoolean(const char *booleanValue)
{
    convertToUpperCase((char *)booleanValue);

    return (strcmp("TRUE", booleanValue) || strcmp("Y", booleanValue)) ? true : false;
}

NodeClassType identifyNodeClassType(const char *nodeClassTypeString)
{
    switch (hash(nodeClassTypeString))
    {
    case hash("VARIABLE_NODE"):
        return NodeClassType::VARIABLE_NODE;
    case hash("METHOD_NODE"):
        return NodeClassType::METHOD_NODE;
    case hash("OBJECT_NODE"):
        return NodeClassType::OBJECT_NODE;
    case hash("UNRECOGNIZED_NODE"):
    default:
        return NodeClassType::UNRECOGNIZED_NODE;
    }
}

DataType identifyDataType(const char *dataTypeString)
{
    convertToUpperCase((char *)dataTypeString);
    switch (hash(dataTypeString))
    {
    case hash("UNSIGNEDSHORT"):
        return DataType::UnsignedShort;
    case hash("UNSIGNEDINTEGER"):
        return DataType::UnsignedInteger;
    case hash("UNSIGNEDLONG"):
        return DataType::UnsignedLong;
    case hash("SIGNEDSHORT"):
        return DataType::SignedShort;
    case hash("SIGNEDINTEGER"):
        return DataType::SignedInteger;
    case hash("SIGNEDLONG"):
        return DataType::SignedLong;
    case hash("DOUBLE"):
        return DataType::Double;
    case hash("BOOLEAN"):
        return DataType::Boolean;
    case hash("STRING"):
        return DataType::String;
    case hash("UNKNOWN"):
    default:
        return DataType::Unknown;
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

BaseNodeDescription setUpBaseNodeDescriptor(const char *nodeClassType, const char *writableFlag,
                                            const char *locale, const char *uniqueId, const char *displayName,
                                            const char *browseName, const char *description)
{
    BaseNodeDescription baseNodeDescritpor;
    baseNodeDescritpor.nodeClass = identifyNodeClassType(nodeClassType);
    baseNodeDescritpor.writableFlag = identifyWritableFlag(writableFlag);
    copyCharArray(locale, baseNodeDescritpor.locale);
    copyCharArray(uniqueId, baseNodeDescritpor.uniqueId);
    copyCharArray(displayName, baseNodeDescritpor.displayName);
    copyCharArray(browseName, baseNodeDescritpor.browseName);
    copyCharArray(description, baseNodeDescritpor.description);

    return baseNodeDescritpor;
}

TEST(NodeInformationTests, BaseNodeDescriptorContentFormatTest)
{
    for (int i = 0; i < MAX_BASE_NODE_DESCRITPOR_TEST_COUNT; i++)
    {
        BaseNodeDescription baseNodeDescriptor = setUpBaseNodeDescriptor(
            baseNodeDescriptorInformation[i][0],
            baseNodeDescriptorInformation[i][1],
            baseNodeDescriptorInformation[i][2],
            baseNodeDescriptorInformation[i][3],
            baseNodeDescriptorInformation[i][4],
            baseNodeDescriptorInformation[i][5],
            baseNodeDescriptorInformation[i][6]);

        EXPECT_EQ(identifyNodeClassType(baseNodeDescriptorInformation[i][0]), baseNodeDescriptor.nodeClass);
        EXPECT_EQ(identifyBoolean(baseNodeDescriptorInformation[i][1]), baseNodeDescriptor.writableFlag);

        EXPECT_STREQ(baseNodeDescriptorInformation[i][2], baseNodeDescriptor.locale);
        EXPECT_STREQ(baseNodeDescriptorInformation[i][3], baseNodeDescriptor.uniqueId);
        EXPECT_STREQ(baseNodeDescriptorInformation[i][4], baseNodeDescriptor.displayName);
        EXPECT_STREQ(baseNodeDescriptorInformation[i][5], baseNodeDescriptor.browseName);
        EXPECT_STREQ(baseNodeDescriptorInformation[i][6], baseNodeDescriptor.description);
    }
}

TEST(NodeInformationTests, VariableNodeDescriptorContetFormatTest)
{
}

TEST(NodeInformationTests, NodeDescriptorContetFormatTest)
{
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}