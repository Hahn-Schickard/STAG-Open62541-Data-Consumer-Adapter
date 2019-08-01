#ifndef _NODE_DESCRIPTORS_TEST_UTILS_HPP
#define _NODE_DESCRIPTORS_TEST_UTILS_HPP

#include "JsonSerializer.hpp"
#include <gtest/gtest.h>
#include <string>

template <typename T>
T getNodeDescription(const char *node_descriptor_name, std::string test_name) {
  int test_number = std::stoi(test_name.substr(test_name.find("/") + 1));
  T node_descriptor;

  try {
    node_descriptor =
        nlohmann::setUpNodeDescriptors<T>(node_descriptor_name)[test_number];
  } catch (std::exception &ex) {
    std::cerr << "Cought a JSON exception for file: " << node_descriptor_name
              << " for test: " << test_name << " " << ex.what() << std::endl;
  }

  return node_descriptor;
}

std::string getCurrentTestName() {
  const ::testing::TestInfo *const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
  std::string test_name = std::string(test_info->name());
  return test_name;
}

template <typename T>
std::vector<T> getNodeDescriptors(const char *node_descriptors_json_file) {
  std::vector<T> node_descriptors;
  try {
    node_descriptors =
        nlohmann::setUpNodeDescriptors<T>(node_descriptors_json_file);
  } catch (std::exception &ex) {
    std::cerr << "Cought a JSON exception for file: "
              << node_descriptors_json_file << " " << ex.what() << std::endl;
  }

  return node_descriptors;
}

#endif //_NODE_DESCRIPTORS_TEST_UTILS_HPP