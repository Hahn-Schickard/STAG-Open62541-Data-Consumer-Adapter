cmake_minimum_required(VERSION 3.6)

set(THIS Unit_Tests_Runner)

file(GLOB Device_Test_Suite "${CMAKE_CURRENT_LIST_DIR}/Device/*")
file(GLOB Shared_Resources "${CMAKE_CURRENT_LIST_DIR}/*.?pp")

#@+ ====================== User TEST_SUITES configuration ==========================
list(APPEND TEST_SUITES
                       ${Device_Test_Suite}
                       ${Shared_Resources}
)
#@- ========================= END OF USER CONFIGURATION ============================

find_package(GTest REQUIRED)

add_executable(${THIS})
target_sources(${THIS} PRIVATE
                       "testRunner.cpp"
                       ${TEST_SUITES}
)

#@+ ==================== User TEST_DEPENDCIES configuration =======================
list(APPEND TEST_DEPENDCIES
                              OPCUA_Adapter
)
#@- ========================= END OF USER CONFIGURATION ===========================

target_link_libraries(${THIS} PUBLIC
                              ${TEST_DEPENDCIES}
                              GTest::gtest
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

set_target_properties(${THIS} PROPERTIES
                              CXX_STANDARD 17
)
