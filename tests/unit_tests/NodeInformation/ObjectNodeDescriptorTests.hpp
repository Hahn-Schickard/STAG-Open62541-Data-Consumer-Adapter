#ifndef _OBJECT_NODE_DESCRIPTOR_TESTS_HPP
#define _OBJECT_NODE_DESCRIPTOR_TESTS_HPP

#include "NodeInformation.h"
#include "NodeDescriptorsTestUtils.hpp"
#include <gtest/gtest.h>

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;

const char *OBJECT_NODE_DESCRIPTORS_FILE = "test_inputs/ObjectNodeDescriptors.json";
const char *PASSING_OBJECT_NODE_DESCRIPTORS_FILE = "expected_results/PassingObjectNodeDescriptors.json";
const char *FAILLING_OBJECT_NODE_DESCRIPTORS_FILE = "expected_results/FaillingObjectNodeDescriptors.json";

class ObjectNodeDescriptorTests : public ::testing::Test,
                                  public ::testing::WithParamInterface<ObjectNodeDescription>
{
};

INSTANTIATE_TEST_SUITE_P(ParametrizedObjectNodeDescriptorTests, ObjectNodeDescriptorTests,
                         ValuesIn(getNodeDescriptors<ObjectNodeDescription>(OBJECT_NODE_DESCRIPTORS_FILE)));

#endif //_OBJECT_NODE_DESCRIPTOR_TESTS_HPP