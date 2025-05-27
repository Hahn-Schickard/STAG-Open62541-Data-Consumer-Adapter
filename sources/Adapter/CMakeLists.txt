set(MODULE ${PROJECT_NAME})

list(APPEND MODULE_INCLUDES_DIRS
            ${INCLUDES_DIR}/Adapter
)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

INCLUDE_DIRS_AND_FILES("${MODULE_INCLUDES_DIRS}" MODULE_INCLUDES)

add_library(${MODULE})

target_sources(${MODULE} PRIVATE
                       ${MODULE_INCLUDES}
                       ${sources_list}
)

target_link_libraries(${MODULE} PUBLIC
                              Data_Consumer_Adapter_Interface::Data_Consumer_Adapter_Interface
                              Variant_Visitor::Variant_Visitor
                              HaSLL::HaSLL                           
                              ${PROJECT_NAME}_Server
)

target_include_directories(${MODULE} PUBLIC
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/Adapter>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

target_compile_features(${MODULE} PUBLIC cxx_std_17)

PRINT_TARGET_PROPERTIES(${MODULE})

install(
    TARGETS ${MODULE}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES ${MODULE_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
