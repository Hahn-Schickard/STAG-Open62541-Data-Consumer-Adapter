cmake_minimum_required(VERSION 3.24.0)

string(CONCAT CMAKE_CONAN_DESCRIPTION
    "Enables integrated conan package manager module. "
    "(Requires conan 2.x and python3 to be installed)"
)
option(CMAKE_CONAN ${CMAKE_CONAN_DESCRIPTION} ON)

if(CMAKE_CONAN)
    set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake
    )
endif()

#@+ ======================== User PROJECT_NAME configuration ============================
set(THIS Open62541_Data_Consumer_Adapter)
#@- =========================== END OF USER CONFIGURATION ===============================

project(${THIS} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")
set(PUBLIC_INCLUDES_DIR "${INCLUDES_DIR}/public")
set(PRIVATE_INCLUDES_DIR "${INCLUDES_DIR}/private")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VERBOSE_FILE_INCLUSION "Prints all included header files" ON)
option(RUN_TESTS "Enables Unit tests runner (Requires GTest framework)" ON)
option(COVERAGE_TRACKING "Enable code test coverage tracking with gcov" ON)
string(CONCAT ENABLE_RUNTIME_CHECKS_DESCRIPTION
    "Enables various runtime checks to improve reliability and security. "
    "Can impact performance"
)
option(ENABLE_RUNTIME_CHECKS ${ENABLE_RUNTIME_CHECKS_DESCRIPTION} OFF)

if(NOT WIN32)
    option(STATIC_CODE_ANALYSIS "Enable Code Linter" ON)
endif(NOT WIN32)

include(macros)
include(linter)
include(compiler_settings)
include(GNUInstallDirs)

#@+ ======================= User CMAKE_INCLUDES configuration ===========================
# User defined cmake module includes go here
#@- =========================== END OF USER CONFIGURATION ===============================

#@+ ======================== User CMAKE_OPTIONS configuration ===========================
# User defined options and cmake variables are set here
option(HISTORIZATION "Enable historization service, required open62541 with defined UA_ENABLE_HISTORIZING macro value" On)
#@- ===================== END OF USER CONFIGURATION ================================

find_package(GTest REQUIRED)

#@+ ========================== User PACKAGES configuration ==============================
find_package(Data_Consumer_Adapter_Interface REQUIRED)
find_package(Variant_Visitor REQUIRED)
find_package(HaSLL REQUIRED)
find_package(Information_Model_Mocks REQUIRED)
find_package(open62541 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

if(HISTORIZATION)
        find_package(libpqxx REQUIRED)
        find_package(date REQUIRED)
endif()
#@- =========================== END OF USER CONFIGURATION ===============================

if(CMAKE_CONAN)
    set(CONAN_TOOLCHAIN_PATH
        "${CMAKE_BINARY_DIR}/conan/build/${CMAKE_BUILD_TYPE}/generators"
    )
    include(${CONAN_TOOLCHAIN_PATH}/conan_toolchain.cmake)
endif(CMAKE_CONAN)

add_subdirectory(sources)
if(RUN_TESTS)
    enable_testing()
    add_subdirectory(unit_tests)
endif(RUN_TESTS)
