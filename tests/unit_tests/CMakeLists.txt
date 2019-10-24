cmake_minimum_required(VERSION 3.6)

set(THIS Test_Runner)

file(GLOB Test_Suite "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB Test_Cases "${CMAKE_CURRENT_LIST_DIR}/*.hpp")

list(APPEND PUBLIC_INCLUDES_DIRS
            ${INCLUDES_DIR}/OPCUA_Adapter
            ${INCLUDES_DIR}/Open62541_Server
)

INCLUDE_DIRS_AND_FILES("${PUBLIC_INCLUDES_DIRS}" PUBLIC_INCLUDES)

add_executable(${THIS} "${Test_Suite}" "${Test_Cases}" "${PUBLIC_INCLUDES}")

target_link_libraries(${THIS} PUBLIC
    OPCUA_Adapter
    OPCUA_Server
    gtest_main
    gmock_main
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

if(CLANG_TIDY_EXE)               
    set_target_properties(${THIS} PROPERTIES
                                  CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}"
)
endif()    

PRINT_TARGET_PROPERTIES(${THIS})

if(WIN32)
    add_custom_command(TARGET ${THIS} POST_BUILD 
                        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                            "${CMAKE_BINARY_DIR}/bin/"
                            $<TARGET_FILE_DIR:${THIS}> 
                        COMMENT "Copying shared library files from ${CMAKE_BINARY_DIR}/bin directory to ${CMAKE_CURRENT_BINARY_DIR} directory."
                            VERBATIM
    )
endif()
