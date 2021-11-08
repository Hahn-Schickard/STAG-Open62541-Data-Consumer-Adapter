#include "gtest/gtest.h"

#include "LoggerRepository.hpp"

int main(int argc, char **argv) {
  auto config = HaSLL::Configuration(
    "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
    HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
