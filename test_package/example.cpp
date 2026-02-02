#include <HaSLL/LoggerManager.hpp>
#include <Open62541_Data_Consumer_Adapter/Open62541Adapter.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace HaSLL;

int main(int, char* argv[]) {
  auto status = EXIT_SUCCESS;
  try {
    LoggerManager::initialise(makeDefaultRepository());

    auto connector = [](const DataNotifier&) {
      auto connection = make_shared<DataConnection>();
      return connection;
    };

    auto this_dir =
        filesystem::weakly_canonical(filesystem::path(argv[0])).parent_path();
    auto configfile = this_dir / "config" / "defaultConfig.json";
    auto adapter = makeOpen62541Adapter(connector, configfile);

    adapter->start();
    this_thread::sleep_for(1s);
    adapter->stop();
    cout << "Integration test succeeded" << endl;

    LoggerManager::terminate();
  } catch (...) {
    cerr << "Unknown error occurred during program execution" << endl;
    cerr << "Integration test failed" << endl;
    status = EXIT_FAILURE;
  }
  exit(status);
}
