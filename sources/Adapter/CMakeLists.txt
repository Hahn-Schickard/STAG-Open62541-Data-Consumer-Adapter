set(TARGET ${PROJECT_NAME})

#@+ =========================== User TARGET configuration ===============================
set(THIS_INCLUDES_DIR ${INCLUDES_DIR}/Adapter)
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
    PUBLIC
        Data_Consumer_Adapter_Interface::Data_Consumer_Adapter_Interface
        HaSLL::HaSLL
    PRIVATE
        Variant_Visitor::Variant_Visitor
        ${PROJECT_NAME}_Server
)
#@- =========================== END OF USER CONFIGURATION ===============================

target_include_directories(${TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${THIS_INCLUDES_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

PRINT_TARGET_PROPERTIES(${TARGET})

INSTALL_TARGET(${TARGET})
INSTALL_HEADERS(HEADERS "${THIS_INCLUDES}")
