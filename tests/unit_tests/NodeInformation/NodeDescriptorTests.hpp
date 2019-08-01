#ifndef _NODE_DESCRIPTOR_TESTS_HPP_
#define _NODE_DESCRIPTOR_TESTS_HPP_

#include "DeviceBuilder.hpp"
#include "NodeBuilder.hpp"
#include "NodeDescriptorsTestUtils.hpp"
#include "NodeInformation.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *NODE_DESCRIPTORS_FILE = "test_inputs/NodeDescriptors.json";
const char *PASSING_NODE_DESCRIPTORS_FILE =
    "expected_results/NodeDescriptorResults/PassingNodeDescriptors.json";
const char *FAILLING_NODE_DESCRIPTORS_FILE =
    "expected_results/NodeDescriptorResults/FaillingNodeDescriptors.json";

class NodeDescriptorTests
    : public ::testing::Test,
      public ::testing::WithParamInterface<NodeDescription> {};

TEST(NodeDescriptorTests, isDefaultBuildCorrect) {
  std::string test_name = getCurrentTestName();

  Model_Factory::DeviceBuilder *abstraction_builder =
      new Model_Factory::DeviceBuilder("TestDevice", "1234",
                                       "A hardcoded device for testing only");
  abstraction_builder->addDeviceElementGroup(
      "RootGroup", "A root group containign all of the elements within device");
  abstraction_builder->addDeviceElement(
      "SimpleElement", "This is a simple element",
      Information_Model::ElementType::Readonly);

  opcua_model_translator::NodeBuilder *node_builder =
      new opcua_model_translator::NodeBuilder(
          abstraction_builder->getDevice().get());
  NodeDescription *node_descriptor = node_builder->get_Node_Description();

  EXPECT_STREQ("TestDevice", node_descriptor->base_node_descriptor.browse_name)
      << "Browse names are not equal for test:" << test_name << std::endl
      << "expected: "
      << "TestDevice" << std::endl
      << "provided: " << node_descriptor->base_node_descriptor.browse_name
      << std::endl;
  EXPECT_STREQ("A hardcoded device for testing only",
               node_descriptor->base_node_descriptor.description)
      << "Descriptions are not equal for test:" << test_name << std::endl
      << "expected: "
      << "A hardcoded device for testing only" << std::endl
      << "provided: " << node_descriptor->base_node_descriptor.description
      << std::endl;
  EXPECT_STREQ("TestDevice", node_descriptor->base_node_descriptor.display_name)
      << "Display names are not equal for test:" << test_name << std::endl
      << "expected: "
      << "TestDevice" << std::endl
      << "provided: " << node_descriptor->base_node_descriptor.display_name
      << std::endl;
  EXPECT_STREQ("EN_US", node_descriptor->base_node_descriptor.locale)
      << "Locales are not equal for test:" << test_name << std::endl
      << "expected: "
      << "EN_US" << std::endl
      << "provided: " << node_descriptor->base_node_descriptor.locale
      << std::endl;
  EXPECT_EQ(NodeClassType::OBJECT_NODE,
            node_descriptor->base_node_descriptor.node_class)
      << "Node class types are not equal for test:" << test_name << std::endl
      << "expected: " << NodeClassType::OBJECT_NODE << std::endl
      << "provided: " << node_descriptor->base_node_descriptor.node_class
      << std::endl;
  EXPECT_STREQ("1234", node_descriptor->base_node_descriptor.unique_id)
      << "Unique ids are not equal for test:" << test_name << std::endl
      << "expected: "
      << "1234" << std::endl
      << "provided: " << node_descriptor->base_node_descriptor.unique_id
      << std::endl;

  delete abstraction_builder;
  delete node_builder;
}

TEST_P(NodeDescriptorTests, isBaseNodeDescriptorEqual) {
  std::string test_name = getCurrentTestName();

  NodeDescription node_descriptor = getNodeDescription<NodeDescription>(
      PASSING_NODE_DESCRIPTORS_FILE, test_name);
  NodeDescription expected_data = GetParam();

  EXPECT_STREQ(expected_data.base_node_descriptor.browse_name,
               node_descriptor.base_node_descriptor.browse_name)
      << "Browse names are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.browse_name
      << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.browse_name
      << std::endl;
  EXPECT_STREQ(expected_data.base_node_descriptor.description,
               node_descriptor.base_node_descriptor.description)
      << "Descriptions are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.description
      << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.description
      << std::endl;
  EXPECT_STREQ(expected_data.base_node_descriptor.display_name,
               node_descriptor.base_node_descriptor.display_name)
      << "Display names are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.display_name
      << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.display_name
      << std::endl;
  EXPECT_STREQ(expected_data.base_node_descriptor.locale,
               node_descriptor.base_node_descriptor.locale)
      << "Locales are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.locale << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.locale
      << std::endl;
  EXPECT_EQ(expected_data.base_node_descriptor.node_class,
            node_descriptor.base_node_descriptor.node_class)
      << "Node class types are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.node_class
      << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.node_class
      << std::endl;
  EXPECT_STREQ(expected_data.base_node_descriptor.unique_id,
               node_descriptor.base_node_descriptor.unique_id)
      << "Unique ids are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.unique_id
      << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.unique_id
      << std::endl;
  EXPECT_EQ(expected_data.base_node_descriptor.writable_flag,
            node_descriptor.base_node_descriptor.writable_flag)
      << "Writable flags are not equal for test:" << test_name << std::endl
      << "expected: " << expected_data.base_node_descriptor.writable_flag
      << std::endl
      << "provided: " << node_descriptor.base_node_descriptor.writable_flag
      << std::endl;
}

INSTANTIATE_TEST_SUITE_P(
    ParametrizedNodeDescriptorTests, NodeDescriptorTests,
    ValuesIn(getNodeDescriptors<NodeDescription>(NODE_DESCRIPTORS_FILE)));

#endif //_NODE_DESCRIPTOR_TESTS_HPP_