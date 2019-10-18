cmake_minimum_required(VERSION 3.6) 

set(THIS Server_Runner)

list(APPEND PUBLIC_INCLUDES_DIRS
            ${INCLUDES_DIR}/Server_Runner
)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

INCLUDE_DIRS_AND_FILES("${PUBLIC_INCLUDES_DIRS}" PUBLIC_INCLUDES)

add_library(${THIS} SHARED 
                    ${PUBLIC_INCLUDES}
)

target_sources(${THIS} PRIVATE 
                       ${sources_list}
)

target_link_libraries(${THIS} PUBLIC
                              OPCUA_Server
)

if (UNIX)
    target_link_libraries(${THIS} PRIVATE
                                  gcov
    )
endif()

target_include_directories(${THIS} PUBLIC
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/Server_Runner>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Server_Runner>
)

if(CLANG_TIDY_EXE)               
    set_target_properties(
                        ${THIS} PROPERTIES
                        CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}"
    )
endif()    

PRINT_TARGET_PROPERTIES(${THIS})

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
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY ${PUBLIC_INCLUDES_DIRS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)