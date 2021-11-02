#include "gtest/gtest.h"

#include "Open62541Server.hpp"

namespace Open62541ServerTests {

using namespace open62541;

TEST(Open62541ServerTests, defaultConstructorWorks) {
  Open62541Server server;
}

TEST(Open62541ServerTests, defaultConfigurationValid) {
  auto config = std::make_unique<Configuration>();
  Open62541Server server(std::move(config));
}

} // namespace
