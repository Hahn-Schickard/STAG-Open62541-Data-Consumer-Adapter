#include "gtest/gtest.h"
#include "BaseNodeDescriptorTests.hpp"
#include "VariableNodeDescriptorTests.hpp"
#include "ObjectNodeDescriptorTests.hpp"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}