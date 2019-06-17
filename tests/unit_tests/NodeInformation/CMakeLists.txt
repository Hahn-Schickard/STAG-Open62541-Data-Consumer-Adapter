cmake_minimum_required(VERSION 3.6)

set (THIS Node_Information_Test_Suite)

file(GLOB Test_Suite "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB Test_Cases "${CMAKE_CURRENT_LIST_DIR}/*.hpp")
file(GLOB Json_Headers "${json_serializer_includes_dir}/*.hpp")

add_executable(${THIS} ${Test_Suite} ${Test_Cases} )

target_link_libraries(${THIS} PUBLIC
    JSON_Nodes_Serializer
    gtest_main
)

add_custom_command(TARGET ${THIS} POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_CURRENT_LIST_DIR}/config $<TARGET_FILE_DIR:${THIS}>)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

if(WIN32)
    add_custom_command(TARGET ${THIS} POST_BUILD 
                        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                            "${CMAKE_BINARY_DIR}/bin/"
                            $<TARGET_FILE_DIR:${THIS}> 
                        COMMENT "Copying shared library files from ${CMAKE_BINARY_DIR}/bin directory to ${CMAKE_CURRENT_BINARY_DIR} directory."
                            VERBATIM)
endif()
