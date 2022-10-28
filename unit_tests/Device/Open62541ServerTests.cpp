#include "gtest/gtest.h"

#include "Open62541Server.hpp"

namespace Open62541ServerTests {

using namespace open62541;

struct Open62541ServerTests : public ::testing::Test {};

// NOLINTNEXTLINE
TEST_F(Open62541ServerTests, defaultConstructorWorks) {
  Open62541Server server;
}

// NOLINTNEXTLINE
TEST_F(Open62541ServerTests, defaultConfigurationValid) {
  auto config = std::make_unique<Configuration>();
  Open62541Server server(std::move(config));
}

} // namespace Open62541ServerTests
