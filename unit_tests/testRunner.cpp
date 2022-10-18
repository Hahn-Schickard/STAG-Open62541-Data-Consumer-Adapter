#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace HaSLL;

int main(int argc, char** argv) {
  auto repo = make_shared<SPD_LoggerRepository>();
  LoggerManager::initialise(repo);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
