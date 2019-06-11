cmake_minimum_required(VERSION 3.6)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_POSITION_INDEPENDENT_CODE on)
set(VERBOSE_FILE_INCLUSION on)
set(THIS Model_Translator)
set(PROJECT_FLAGS C CXX)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

project(${THIS} ${PROJECT_FLAGS})

enable_testing()

find_program(CLANG_TIDY "clang-tidy")

if(CLANG_TIDY)
set(CMAKE_CXX_CLANG_TIDY  
    clang-tidy;
    -header-filter=.;
    -checks='.*,readability-identifier-naming';
    CheckOptions: 
    - { key: readability-identifier-naming.ClassCase, 
        value: CamelCase}
    - { key: readability-identifier-naming.ClassConstantCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.ClassMethodCase,
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.ConstantCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.ConstantMemberCase, 
        value: Camel_Snake_Case}
    - { key: readability-identifier-naming.ConstantPointerParameterCase, 
        value: aNy_CasE}
    - { key: readability-identifier-naming.ConstantPointerParameterCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.ConstexprFunctionCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.ConstexprFunctionPrefix, 
        value: CExpr_}
    - { key: readability-identifier-naming.ConstexprMethodCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.ConstexprMethodPrefix, 
        value: CExpr_}
    - { key: readability-identifier-naming.ConstexprVariableCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.EnumCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.EnumConstantCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.FunctionCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.GlobalConstantCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.GlobalConstantPointerCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.GlobalFunctionCase, 
        value: Camel_Snake_Case}
    - { key: readability-identifier-naming.GlobalFunctionPrefix, 
        value: GLB_FUNC_}
    - { key: readability-identifier-naming.GlobalPointerCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.GlobalVariableCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.LocalConstantCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.LocalConstantPointerCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.LocalPointerCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.LocalVariableCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.MemberCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.MethodCase, 
        value: camelBack}
    - { key: readability-identifier-naming.NamespaceCase, 
        value: camelBack}
    - { key: readability-identifier-naming.ParameterCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.ParameterPackCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.PrivateMemberCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.PrivateMethodCase, 
        value: camelBack}
    - { key: readability-identifier-naming.ProtectedMethodCase, 
        value: camelBack}
    - { key: readability-identifier-naming.PublicMemberCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.PublicMethodCase, 
        value: camelBack}
    - { key: readability-identifier-naming.StaticConstantCase, 
        value: UPPER_CASE}
    - { key: readability-identifier-naming.StaticVariableCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.StructCase, 
        value: CamelCase}
    - { key: readability-identifier-naming.TypedefCase, 
        value: CamelCase}
    - { key: readability-identifier-naming.UnionCase, 
        value: CamelCase}
    - { key: readability-identifier-naming.VariableCase, 
        value: camel_Snake_Back}
    - { key: readability-identifier-naming.VirtualMethodCase, 
        value: camelBack}
    -std=c++11)
endif()

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

# ==============================================================================================================================

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