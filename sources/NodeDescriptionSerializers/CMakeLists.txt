cmake_minimum_required(VERSION 3.6)

set (THIS JSON_Nodes_Serializer)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB includes_list  "${PROJECT_SOURCE_DIR}/includes/NodeInformation/*.h"
                        "${PROJECT_SOURCE_DIR}/includes/utilities/JsonSerializer/*.hpp")

PRINT_MODULE_FILES(${THIS} "${sources_list}" "${includes_list}")

add_library(${THIS} SHARED
            ${sources_list}
            ${includes_list})

target_link_libraries(${THIS} PUBLIC 
                      C_String_Formater
                      Data_Value_Identifier
                      nlohmann_json::nlohmann_json
                      gcov)

target_include_directories(${THIS} INTERFACE 
                           ${json_serializer_includes_dir})

add_custom_command(TARGET ${THIS} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E make_directory 
            ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:${THIS}> 
            "${CMAKE_BINARY_DIR}/bin"
        COMMENT "Copying shared library files from ${CMAKE_CURRENT_BINARY_DIR} directory to ${CMAKE_BINARY_DIR}/bin directory."
            VERBATIM)         
