#@+ ======================== User TARGET NAME configuration ============================
set(THIS Example)
#@- =========================== END OF USER CONFIGURATION ===============================

set(TARGET ${PROJECT_NAME}_${THIS})

#@+ =========================== User TARGET configuration ===============================
file(GLOB SOURCEFILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(${TARGET})

target_sources(${TARGET}
    PRIVATE
        ${SOURCEFILES}
)

target_link_libraries(${TARGET}
    PUBLIC 
        ${PROJECT_NAME}_Adapter
        Information_Model_Mocks::Information_Model_Mocks
        Variant_Visitor::Variant_Visitor
)

add_custom_command(TARGET ${TARGET} POST_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${PROJECT_SOURCE_DIR}/config
                        $<TARGET_FILE_DIR:${TARGET}>/config 
                    COMMENT "Copying configuration files."
                        VERBATIM
)
#@- =========================== END OF USER CONFIGURATION ===============================

IMPORT_TARGET_DLLS(${TARGET})
