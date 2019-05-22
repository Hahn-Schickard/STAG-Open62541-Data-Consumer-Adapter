cmake_minimum_required(VERSION 3.6)

set (THIS Model_Translator)
set (PROJECT_FLAGS C CXX)

project(${THIS} ${PROJECT_FLAGS})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE on)l

enable_testing()

find_package(Threads REQUIRED)

include(ExternalProject)

ExternalProject_Add(googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           master
    SOURCE_DIR        "${PROJECT_SOURCE_DIR}/tests/googletest-src"
    BINARY_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
)

include_directories("${PROJECT_SOURCE_DIR}/includes/NodeInformation")
include_directories("${PROJECT_SOURCE_DIR}/includes/utilities")

file(GLOB C_Includes "${PROJECT_SOURCE_DIR}/includes/NodeInformation/*.h")
file(GLOB CXX_Includes  "${PROJECT_SOURCE_DIR}/includes/*.hpp"
                        "${PROJECT_SOURCE_DIR}/includes/utilities/*.hpp")

add_subdirectory(sources)
add_subdirectory(tests/unit_tests)