#@+ =================== User TEST SUIT TARGET NAME configuration ========================
set(THIS Unit_Tests_Runner)
#@- =========================== END OF USER CONFIGURATION ===============================

#@+ ======================== User TEST_SUITES configuration =============================
file(GLOB Device_Test_Suite "${CMAKE_CURRENT_LIST_DIR}/Device/*")
file(GLOB Shared_Resources "${CMAKE_CURRENT_LIST_DIR}/*.?pp")
list(APPEND TEST_SUITES
    ${Device_Test_Suite}
    ${Shared_Resources}
)
#@- =========================== END OF USER CONFIGURATION ===============================
add_executable(${THIS})

target_sources(${THIS}
    PRIVATE
        "testRunner.cpp"
        ${TEST_SUITES}
)
#@+ ======================= User TEST_DECENCIES configuration ===========================
list(APPEND TEST_DECENCIES
    ${PROJECT_NAME}_Adapter
    Information_Model_Mocks::Information_Model_Mocks
)
#@- =========================== END OF USER CONFIGURATION ===============================
target_link_libraries(${THIS}
    PRIVATE
        GTest::gtest
        GTest::gmock
        ${TEST_DECENCIES}
)

add_test(
    NAME ${THIS}
    COMMAND ${THIS}
)

set_target_properties(${THIS}
    PROPERTIES
        CXX_STANDARD 17
)

PRINT_TARGET_PROPERTIES(${THIS})

IMPORT_TARGET_DLLS(${THIS})
