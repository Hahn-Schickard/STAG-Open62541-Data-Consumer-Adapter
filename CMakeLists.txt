cmake_minimum_required(VERSION 3.6)

#@+ ========================== User configuration ===========================
set(THIS Open62541_Data_Consumer_Adapter)
#@- ===================== END OF USER CONFIGURATION =========================

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_POSITION_INDEPENDENT_CODE on)
set(VERBOSE_FILE_INCLUSION on)
set(PROJECT_FLAGS C CXX)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

project(${THIS} ${PROJECT_FLAGS})

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(WARNING "No toolchain specified, defaulting to native one!")
    set(CMAKE_TOOLCHAIN_FILE cmake/native-toolchain.cmake)
endif()

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_STATIC_CODE_ANALYSIS ON CACHE BOOL "Activates static code analysis during the build")
set(PACKAGE_MAJOR_VER "0" CACHE STRING "Major version number of the release package. This option is overriden by CI/CD setup, however for local builds it is still valid")
set(PACKAGE_MINOR_VER "0" CACHE STRING "Major version number of the release package. This option is overriden by CI/CD setup, however for local builds it is still valid")
set(PACKAGE_PATCH_VER "0" CACHE STRING "Major version number of the release package. This option is overriden by CI/CD setup, however for local builds it is still valid")
set(THIS_PACKAGE_NAME "${PROJECT_NAME}-${PACKAGE_MAJOR_VER}.${PACKAGE_MINOR_VER}.${PACKAGE_PATCH_VER}-${CMAKE_SYSTEM_NAME}")
set(CLANG_OUTPUT "${PROJECT_SOURCE_DIR}/clang-tidy-suggested-fixes.yml")
set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")

#@+ ========================== User configuration ===========================
find_package(Information_Modeler REQUIRED PATHS "./modules")
find_package(Model_Event_Handler REQUIRED PATHS "./modules")
find_package(open62541 REQUIRED PATHS "./modules")
#@- ===================== END OF USER CONFIGURATION =========================

set(CMAKE_CONFIGURATION_TYPES "DEBUG" CACHE STRING "Configs" FORCE)
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_VERSION VERSION_GREATER "2.8")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS  ${CMAKE_CONFIGURATION_TYPES})
endif()

include(GNUInstallDirs)
include(ClangFormat)
include(HunterGate)
include(macros)
include(compiler_settings)
include(linter)

#@+ ========================== User configuration ===========================
set(PROJECT_MAINTAINERS "Dovydas Girdvainis")

include(dependencies)

include_directories(${INCLUDES_DIR})

add_subdirectory(sources)

add_subdirectory(tests/unit_tests/)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is an example project with template files.")
set(CPACK_PACKAGE_VENDOR "Hahn-Schickard")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_CONTACT "dovydas.girdvainis@hahn-schickard.de")
#@- ===================== END OF USER CONFIGURATION =========================

set(CPACK_PACKAGE_VERSION_MAJOR ${PACKAGE_MAJOR_VER})
set(CPACK_PACKAGE_VERSION_MINOR ${PACKAGE_MINOR_VER})
set(CPACK_PACKAGE_VERSION_PATCH ${PACKAGE_PATCH_VER})
set(CPACK_OUTPUT_FILE_PREFIX cpack_packages)
set(PACKAGE_VERSION "${PACKAGE_MAJOR_VER}.${PACKAGE_MINOR_VER}.${PACKAGE_PATCH_VER}")		
set(CONFIGS_DEST "cmake")

WRITE_PKG_CONFIG_IN_FILE(${PROJECT_NAME})		
        
include(CMakePackageConfigHelpers)		
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"		
                                 VERSION ${PACKAGE_VERSION}		
                                 COMPATIBILITY SameMajorVersion
)		
        
configure_package_config_file(		
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in"		
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"		
  INSTALL_DESTINATION ${THIS_PACKAGE_NAME}/
)		

install(EXPORT ${PROJECT_NAME}Targets		
        FILE ${PROJECT_NAME}Targets.cmake		
        NAMESPACE ${PROJECT_NAME}::		
        DESTINATION ${CONFIGS_DEST}
)
        
install(FILES 
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake		
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake		
        DESTINATION ${CONFIGS_DEST}
)

install(FILES 
        README.md	
        DESTINATION ${CONFIGS_DEST}/..
)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/docs/code_documentation/" DESTINATION "docs")

if(WIN32 AND NOT UNIX)
    message(STATUS "Using ZIP archiving format for packages!")
    set(CPACK_GENERATOR "ZIP")
elseif(UNIX AND NOT WIN32)
    if(DEBIAN_PACKAGING)
        message(STATUS "Using DEB archiving format for packages!")
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${PROJECT_MAINTAINERS})
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Linux-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
    else()
        find_program(BZip2 NAMES "bzip2")
        if(BZIP2_FOUND)
            message(STATUS "Using BZip2 archiving format for packages!")
            set(CPACK_GENERATOR "TBZ2")
        else()
            message(STATUS "Using TGZ archiving format for packages!")
            set(CPACK_GENERATOR "TGZ")
        endif()
    endif()
else()
    message(WARNING "Packaging has not been configured for your Operating System!")
endif()

include(CPack)
