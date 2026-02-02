#@+ ======================== User TARGET NAME configuration =============================
set(THIS Historizer)
#@- =========================== END OF USER CONFIGURATION ===============================

set(TARGET ${PROJECT_NAME}_${THIS})

#@+ =========================== User TARGET configuration ===============================
set(THIS_INCLUDES_DIR ${INCLUDES_DIR}/Historizer)
file(GLOB THIS_INCLUDES ${THIS_INCLUDES_DIR}/*.hpp)
file(GLOB SOURCEFILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_library(${TARGET} OBJECT)
target_sources(${TARGET}
    PUBLIC
        ${THIS_INCLUDES}
    PRIVATE
        ${SOURCEFILES}
)

target_link_libraries(${TARGET} 
    PUBLIC
        libpqxx::pqxx
        open62541::open62541
        HaSLL::HaSLL
    PRIVATE
        date::date
        fmt::fmt-header-only
        ${PROJECT_NAME}_Utilities
)
#@- =========================== END OF USER CONFIGURATION ===============================

target_include_directories(${TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${THIS_INCLUDES_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

PRINT_TARGET_PROPERTIES(${TARGET})
