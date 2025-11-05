#@+ ======================== User TARGET NAME configuration =============================
set(THIS Server)
#@- =========================== END OF USER CONFIGURATION ===============================

set(TARGET ${PROJECT_NAME}_${THIS})

#@+ =========================== User TARGET configuration ===============================
set(THIS_INCLUDES_DIR ${INCLUDES_DIR}/Server)
file(GLOB THIS_INCLUDES ${THIS_INCLUDES_DIR}/*.hpp)
file(GLOB SOURCEFILES "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_library(${TARGET})
target_sources(${TARGET}
    PUBLIC
        ${THIS_INCLUDES}
    PRIVATE
        ${SOURCEFILES}
)

target_link_libraries(${TARGET} 
    PUBLIC #@todo change to private
        Information_Model::Information_Model
        open62541::open62541
        Variant_Visitor::Variant_Visitor
        HaSLL::HaSLL                             
)

if(HISTORIZATION)
    target_link_libraries(${TARGET} 
        PUBLIC #@todo change to private
            date::date
            libpqxx::pqxx
    )
    target_compile_definitions(${TARGET} PUBLIC UA_ENABLE_HISTORIZING)
endif()

#@- =========================== END OF USER CONFIGURATION ===============================

target_include_directories(${TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${THIS_INCLUDES_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

PRINT_TARGET_PROPERTIES(${TARGET})
