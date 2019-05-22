cmake_minimum_required(VERSION 3.6)

set (THIS Model_Translator)
set (PROJECT_FLAGS C CXX)

project(${THIS} ${PROJECT_FLAGS})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE on)l

enable_testing()

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tests/googletest-src"
                 "${CMAKE_BINARY_DIR}/googletest-build")

if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()


include_directories("${PROJECT_SOURCE_DIR}/includes/NodeInformation")
include_directories("${PROJECT_SOURCE_DIR}/includes/utilities")

file(GLOB C_Includes "${PROJECT_SOURCE_DIR}/includes/NodeInformation/*.h")
file(GLOB CXX_Includes  "${PROJECT_SOURCE_DIR}/includes/*.hpp"
                        "${PROJECT_SOURCE_DIR}/includes/utilities/*.hpp")

add_subdirectory(sources)
add_subdirectory(tests/unit_tests)