cmake_minimum_required(VERSION 3.6)

set (THIS OPCUA_Adapter)

file(GLOB sources_list  "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB includes_list "${PROJECT_SOURCE_DIR}/includes/Opcua_Adapter/*.hpp")

PRINT_MODULE_FILES(${THIS} "${sources_list}" "${includes_list}")

set(opcua_adapter_includes_dir "${PROJECT_SOURCE_DIR}/includes/Opcua_Adapter")
include_directories(${opcua_adapter_includes_dir})

add_library(${THIS} SHARED
            ${includes_list})

target_sources(${THIS} PRIVATE ${sources_list})

target_link_libraries(${THIS} PUBLIC
                              Notifier
                              Information_Model 
                              PRIVATE                              
                              Server_Runner)
                              
if (UNIX)
    target_link_libraries(${THIS} PRIVATE
                                  gcov)
endif()


target_include_directories(${THIS} PUBLIC
                           $<BUILD_INTERFACE:${consumer_notifier_includes}>
                           $<BUILD_INTERFACE:${opcua_adapter_includes_dir}>
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
