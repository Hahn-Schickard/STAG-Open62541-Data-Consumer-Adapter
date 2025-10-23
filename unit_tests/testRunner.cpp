#include <HaSLL/LoggerManager.hpp>
#include <gtest/gtest.h>

using namespace std;
using namespace HaSLL;

int main(int argc, char** argv) {
  LoggerManager::initialise(makeDefaultRepository());

  testing::InitGoogleTest(&argc, argv);
  auto status = RUN_ALL_TESTS();

  LoggerManager::terminate();
  return status;
}
