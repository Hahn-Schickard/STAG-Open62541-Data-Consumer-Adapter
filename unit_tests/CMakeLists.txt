cmake_minimum_required(VERSION 3.6)

set(THIS Unit_Tests_Runner)

file(GLOB Device_Test_Suite "${CMAKE_CURRENT_LIST_DIR}/*")

find_package(GTest REQUIRED)

add_executable(${THIS})
target_sources(${THIS} PRIVATE 
                       "testRunner.cpp"
                       ${Device_Test_Suite}
)

target_link_libraries(${THIS} PUBLIC
                              OPCUA_Adapter
                              GTest::GTest
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

set_target_properties(${THIS} PROPERTIES
                              CXX_STANDARD 17
)