cmake_minimum_required(VERSION 3.6)

set (THIS Node_Information_Test_Suite)

file(GLOB Test_Cases "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(${THIS} ${Test_Cases})

target_link_libraries(${THIS} PUBLIC
    gtest_main
    Node_Descriptor_Builder
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)