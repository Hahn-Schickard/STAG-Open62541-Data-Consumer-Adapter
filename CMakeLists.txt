cmake_minimum_required(VERSION 3.6)

set(CMAKE_VERBOSE_MAKEFILE on)
set(VERBOSE_FILE_INCLUSION on)

# Pass your lists in quations, for example:
# PRINT_FILES("${file_list}")
MACRO(PRINT_FILES file_list)
    foreach(thing ${file_list})
        message(STATUS " |-- ${thing}")
    endforeach()
    message("----------------------------------------------------------------------------------------------------------------")
    message(" ")
ENDMACRO()

MACRO(PRINT_MODULE_FILES module_name sources_list includes_list)
    if(VERBOSE_FILE_INCLUSION)
        message(STATUS "${module_name} library files are:")
        message(STATUS "=====Source Files=====")
        PRINT_FILES("${sources_list}")
        message(STATUS "=====Include Files=====")
        PRINT_FILES("${includes_list}")
    endif()
ENDMACRO()

set (THIS Model_Translator)
set (PROJECT_FLAGS C CXX)

project(${THIS} ${PROJECT_FLAGS})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE on)

enable_testing()

configure_file(JsonDependency.txt json-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/json-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/json-download" )
set(JSON_INCLUDES "${PROJECT_SOURCE_DIR}/libs/json-src/single_include/")
set(JSON_BuildTests OFF CACHE INTERNAL "")

configure_file(GoogleTestDependency.txt googletest-download/CMakeLists.txt)
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