#include "SharedTestResources.hpp"

#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace HaSLL;
using namespace open62541;

int main(int argc, char** argv) {
  auto repo = make_shared<SPD_LoggerRepository>();
  LoggerManager::initialise(repo);

  shared_server = make_shared<Open62541Server>();
  shared_node_builder = make_shared<NodeBuilder>(shared_server);

  testing::InitGoogleTest(&argc, argv);
  auto status = RUN_ALL_TESTS();

  shared_server.reset();
  shared_node_builder.reset();
  LoggerManager::terminate();

  return status;
}
