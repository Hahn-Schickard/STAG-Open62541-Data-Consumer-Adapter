cmake_minimum_required(VERSION 3.6)

#@+ ========================== User configuration ===========================
set(THIS Model_Translator)
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

enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FORCE_WINDOWS_LINTING OFF CACHE BOOL "Force linting with clang-tidy on Windows")
set(DEBIAN_PACKAGING OFF CACHE BOOL "Packages files in .deb format.")
set(PACKAGE_MAJOR_VER "0" CACHE STRING "Major version number of the release package. This option is overriden by CI/CD setup, however for local builds it is still valid")
set(PACKAGE_MINOR_VER "0" CACHE STRING "Major version number of the release package. This option is overriden by CI/CD setup, however for local builds it is still valid")
set(PACKAGE_PATCH_VER "0" CACHE STRING "Major version number of the release package. This option is overriden by CI/CD setup, however for local builds it is still valid")
set(CLANG_OUTPUT "${PROJECT_SOURCE_DIR}/clang-tidy-suggested-fixes.yml")
set(INCLUDES_DIR "${PROJECT_SOURCE_DIR}/includes")
set(INCLUDES_DEST "include/")
set(LIBS_DEST "lib/")
set(BINS_DEST "bin/")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "./" CACHE PATH "Specifies where the cpacked binaries and related fills will be installed." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_CONFIGURATION_TYPES "DEBUG" CACHE STRING "Configs" FORCE)
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_VERSION VERSION_GREATER "2.8")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS  ${CMAKE_CONFIGURATION_TYPES})
endif()

include(macros)
include(compiler_settings)
include(linter)

#@+ ========================== User configuration ===========================
set(PROJECT_MAINTAINERS "Dovydas Girdvainis")

include(dependencies)

include_directories(${INCLUDES_DIR})

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

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is an example project with template files.")
set(CPACK_PACKAGE_VENDOR "Hahn-Schickard")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_CONTACT "dovydas.girdvainis@hahn-schickard.de")
#@- ===================== END OF USER CONFIGURATION =========================

set(CPACK_PACKAGE_VERSION_MAJOR ${PACKAGE_MAJOR_VER})
set(CPACK_PACKAGE_VERSION_MINOR ${PACKAGE_MINOR_VER})
set(CPACK_PACKAGE_VERSION_PATCH ${PACKAGE_PATCH_VER})
set(CPACK_OUTPUT_FILE_PREFIX cpack_packages)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/docs/code_documentation/" DESTINATION "docs")
install(EXPORT ${PROJECT_NAME} DESTINATION ${LIBS_DEST})

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
