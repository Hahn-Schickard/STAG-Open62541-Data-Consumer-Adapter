#@+ ======================== User MODULE NAME configuration ============================
set(THIS Serializer)
#@- =========================== END OF USER CONFIGURATION ===============================

set(MODULE ${PROJECT_NAME}_${THIS})

list(APPEND THIS_INCLUDES_DIRS
            ${INCLUDES_DIR}/Serializer
)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

INCLUDE_DIRS_AND_FILES("${THIS_INCLUDES_DIRS}" THIS_INCLUDES)

add_library(${MODULE})

target_sources(${MODULE} PRIVATE 
                       ${THIS_INCLUDES}
                       ${sources_list}
)

target_link_libraries(${MODULE} PUBLIC
                              open62541::open62541
                              nlohmann_json::nlohmann_json
                              HaSLL::HaSLL   
)

target_include_directories(${MODULE} PUBLIC
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/Serializer>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
)

set_property(TARGET ${MODULE} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_features(${MODULE} PUBLIC cxx_std_17)

PRINT_TARGET_PROPERTIES(${MODULE})

install(
    TARGETS ${MODULE}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES ${THIS_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
