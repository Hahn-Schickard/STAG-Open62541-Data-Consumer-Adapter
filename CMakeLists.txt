cmake_minimum_required(VERSION 3.6)

#@+ ========================== User configuration ===========================
set(THIS Open62541_Data_Consumer_Adapter)
#@- ===================== END OF USER CONFIGURATION =========================

project(${THIS} CXX)

enable_testing()

set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(DOCS_DIR "${PROJECT_SOURCE_DIR}/docs/code_documentation")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VERBOSE_FILE_INCLUSION ON)

include(GNUInstallDirs)
include(macros)
include(conan_install)
include(compiler_settings)
execute_conan_install()

#@+ ======================== User PACKAGES configuration ===========================
list(APPEND DEPENDENCIES_LIST 
            HaSLL
            Variant_Visitor
            Data_Consumer_Adapter_Interface
            open62541
            nlohmann_json
            Threads
)
#@- ========================= END OF USER CONFIGURATION ============================

FIND_INSTALLED_DEPENDENCIES("${DEPENDENCIES_LIST}")

add_subdirectory(sources)
add_subdirectory(unit_tests)

WRITE_PKG_CONFIG_IN_FILE(${PROJECT_NAME} "${TRANSITIVE_DEPENENCIES}")		
        
include(CMakePackageConfigHelpers)	

configure_package_config_file(		
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in"		
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"		
  INSTALL_DESTINATION ${THIS_PACKAGE_NAME}/
)	

install(EXPORT ${PROJECT_NAME}Targets		
        FILE ${PROJECT_NAME}Targets.cmake		
        NAMESPACE ${PROJECT_NAME}::		
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(FILES 
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)
