#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"

#include "SharedTestResources.hpp"
#include "gtest/gtest.h"

using namespace std;
using namespace HaSLL;

shared_ptr<open62541::Open62541Server> shared_server;
std::shared_ptr<open62541::NodeBuilder> shared_node_builder;

int main(int argc, char** argv) {
  auto repo = make_shared<SPD_LoggerRepository>();
  LoggerManager::initialise(repo);

  auto logger = LoggerManager::registerLogger("Test Runner");

  logger->info("Registering shared test results");
  shared_server = std::make_shared<open62541::Open62541Server>();
  shared_node_builder = std::make_shared<open62541::NodeBuilder>(shared_server);

  logger->info("Initializing Google Tests");
  testing::InitGoogleTest(&argc, argv);

  logger->info("Running tests");
  auto status = RUN_ALL_TESTS();

  logger->info("Tests finished, cleaning up shared resources");
  shared_server.reset();
  shared_node_builder.reset();

  LoggerManager::terminate();
  return status;
}
