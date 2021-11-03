#include "gtest/gtest.h"

#include "LoggerRepository.hpp"
#include "Open62541Server.hpp"

namespace Open62541ServerTests {

using namespace open62541;

struct Open62541ServerTests : public ::testing::Test {
  Open62541ServerTests() {
    auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
    HaSLL::LoggerRepository::initialise(config);
  }
};


TEST_F(Open62541ServerTests, defaultConstructorWorks) {
  Open62541Server server;
}

TEST_F(Open62541ServerTests, defaultConfigurationValid) {
  auto config = std::make_unique<Configuration>();
  Open62541Server server(std::move(config));
}

} // namespace
