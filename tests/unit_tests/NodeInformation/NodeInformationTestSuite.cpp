#include "BaseNodeDescriptorTests.hpp"
#include "NodeDescriptorTests.hpp"
#include "ObjectNodeDescriptorTests.hpp"
#include "VariableNodeDescriptorTests.hpp"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}