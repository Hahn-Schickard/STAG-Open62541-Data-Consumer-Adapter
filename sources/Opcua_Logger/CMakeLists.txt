cmake_minimum_required(VERSION 3.6)

set(THIS OPCUA_Logger)

list(APPEND PUBLIC_INCLUDES_DIRS
            ${PROJECT_SOURCE_DIR}/includes/Opcua_Logger
)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

INCLUDE_DIRS_AND_FILES(PUBLIC_INCLUDES_DIRS PUBLIC_INCLUDES)
PRINT_MODULE_FILES(${THIS} "${sources_list}" "${PUBLIC_INCLUDES}")

add_library(${THIS} SHARED 
                    ${includes_list}
)

target_sources(${THIS} PRIVATE 
                       ${sources_list}
)

target_link_libraries(${THIS} PUBLIC
                              open62541::open62541
)

if (UNIX)
    target_link_libraries(${THIS} PRIVATE
                                  gcov
    )
endif()

target_include_directories(${THIS} PUBLIC
                           $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/includes/Opcua_Logger>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${INCLUDES_DEST}>
                           $<INSTALL_INTERFACE:${LIBS_DEST}>
)

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
            VERBATIM
) 

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