cmake_minimum_required(VERSION 3.6)

set (THIS Node_Builder)

file(GLOB sources_list  "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB includes_list "${PROJECT_SOURCE_DIR}/includes/NodeBuilder/*.hpp")

PRINT_MODULE_FILES(${THIS} "${sources_list}" "${includes_list}")

set(node_builder_includes_dir "${PROJECT_SOURCE_DIR}/includes/NodeBuilder")
include_directories(${node_builder_includes_dir})

add_library(${THIS} SHARED
            ${sources_list}
            ${includes_list})

if (WIN32)
    target_link_libraries(${THIS} PRIVATE 
                              Node_Information
                              C_String_Formater
                              Information_Model)
elseif (UNIX)
    target_link_libraries(${THIS} PRIVATE 
                              Node_Information
                              C_String_Formater
                              Information_Model
                              gcov)
else ()
    target_link_libraries(${THIS} PRIVATE 
                              Node_Information
                              C_String_Formater
                              Information_Model)
    message(WARNING "Please configure your linking for your operating system for target ${THIS}")
endif ()


target_include_directories(${THIS} INTERFACE
                           $<BUILD_INTERFACE:${node_builder_includes_dir}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${INCLUDES_DEST}>
                           $<INSTALL_INTERFACE:${LIBS_DEST}>)

if(CLANG_TIDY_EXE AND DO_LINTING)               
    set_target_properties(
                        ${THIS} PROPERTIES
                        CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY}
                        )
endif()    

add_custom_command(TARGET ${THIS} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E make_directory 
            ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:${THIS}> 
            "${CMAKE_BINARY_DIR}/bin"
        COMMENT "Copying shared library files from ${CMAKE_CURRENT_BINARY_DIR} directory to ${CMAKE_BINARY_DIR}/bin directory."
            VERBATIM) 

install(
    TARGETS ${THIS} 
    EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LIBS_DEST}
    LIBRARY DESTINATION ${LIBS_DEST}
    COMPONENT libraries
)

install(
    FILES ${includes_list}
    DESTINATION ${INCLUDES_DEST}
    COMPONENT headers
)
