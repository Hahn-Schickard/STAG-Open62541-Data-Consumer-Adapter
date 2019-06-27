cmake_minimum_required(VERSION 3.6)

set(THIS Model_Translator)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_POSITION_INDEPENDENT_CODE on)
set(VERBOSE_FILE_INCLUSION on)
set(PROJECT_FLAGS C CXX)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

project(${THIS} ${PROJECT_FLAGS})

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(IS_SUBMODULE_PROJECT OFF CACHE BOOL "Specifies wheather this project is a submodule for another project")
set(FORCE_WINDOWS_LINTING OFF CACHE BOOL "Force linting with clang-tidy on Windows")
set(CLANG_OUTPUT "${PROJECT_SOURCE_DIR}/clang-tidy-suggested-fixes.yml")
set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")

include(macros)
include(compiler_settings)
include(linter)
include(dependencies)

set(node_information_includes_dir "${PROJECT_SOURCE_DIR}/includes/NodeInformation")
set(utilities_includes_dir "${PROJECT_SOURCE_DIR}/includes/utilities")
set(json_serializer_includes_dir "${PROJECT_SOURCE_DIR}/includes/utilities/JsonSerializer")

include_directories(${node_information_includes_dir})
include_directories(${utilities_includes_dir})
include_directories(${json_serializer_includes_dir})

file(GLOB C_Includes "${PROJECT_SOURCE_DIR}/includes/NodeInformation/*.h")
file(GLOB CXX_Includes  "${PROJECT_SOURCE_DIR}/includes/*.hpp"
                        "${PROJECT_SOURCE_DIR}/includes/utilities/*.hpp")

add_subdirectory(libs/json-src)        
add_subdirectory(sources)
add_subdirectory(tests/unit_tests/NodeInformation)