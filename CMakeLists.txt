cmake_minimum_required(VERSION 3.24.0)

option(CMAKE_CONAN "Enables integeated conan package manager module. (Requires conan and python3 to be installed)" ON)

if(CMAKE_CONAN)
        set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake)
endif()

#@+ ======================== User PROJECT_NAME configuration ============================
set(THIS Open62541_Data_Consumer_Adapter)
#@- =========================== END OF USER CONFIGURATION ===============================

project(${THIS} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(DOCS_DIR "${PROJECT_SOURCE_DIR}/docs/code_documentation")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VERBOSE_FILE_INCLUSION "Prints all included header files" ON)
option(STATIC_CODE_ANALYSIS "Enable Code Linter" ON)
option(RUN_TESTS "Enables Unit tests runner (Requires GTest framework)" ON)
option(COVERAGE_TRACKING "Enable code test coverage tracking with gcov" ON)

include(GNUInstallDirs)
include(macros)
include(linter)
include(compiler_settings)

#@+ ====================== User CMAKE_OPTIONS configuration ========================
# User defined options and cmake variables are set here
option(HISTORIZATION "Enable historization service, required open62541 with defined UA_ENABLE_HISTORIZING macro value" ON)
#@- ===================== END OF USER CONFIGURATION ================================

#@+ ==================== User CMAKE_INCLUDES configuration =======================
# User defined cmake module includes go here
#@- ======================== END OF USER CONFIGURATION ============================

#@+ ======================== User PACKAGES configuration ===========================
find_package(Data_Consumer_Adapter_Interface REQUIRED)
find_package(HaSLL REQUIRED)
find_package(open62541 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)

if(HISTORIZATION)
        find_package(OODD REQUIRED)
        find_package(date REQUIRED)
endif()
#@- ========================= END OF USER CONFIGURATION ============================

add_subdirectory(sources)
if(RUN_TESTS)
        enable_testing()
        add_subdirectory(unit_tests)
endif()
