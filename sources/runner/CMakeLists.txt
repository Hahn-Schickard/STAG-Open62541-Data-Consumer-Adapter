cmake_minimum_required(VERSION 3.6) 

set(THIS Runner)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(${THIS})              

target_sources(${THIS} PRIVATE 
                       ${sources_list}
)

target_link_libraries(${THIS} PRIVATE
                              Information_Modeler::Information_Model_Interface
                              OPCUA_Adapter
)

if(UNIX)
    target_link_libraries(${THIS} PRIVATE 
                                  gcov
    )
endif()

iif(CLANG_TIDY_EXE)               
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