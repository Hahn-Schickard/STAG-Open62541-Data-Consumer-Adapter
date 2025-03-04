cmake_minimum_required(VERSION 3.6) 

set(THIS Config_Serializer)

list(APPEND THIS_INCLUDES_DIRS
            ${INCLUDES_DIR}/Serializer
)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

INCLUDE_DIRS_AND_FILES("${THIS_INCLUDES_DIRS}" THIS_INCLUDES)

add_library(${THIS})

target_sources(${THIS} PRIVATE 
                       ${THIS_INCLUDES}
                       ${sources_list}
)

target_link_libraries(${THIS} PUBLIC
                              open62541::open62541
                              nlohmann_json::nlohmann_json
                              HaSLL::HaSLL   
)

target_include_directories(${THIS} PUBLIC
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/Serializer>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>
)

set_property(TARGET ${THIS} PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_features(${THIS} PUBLIC cxx_std_17)

PRINT_TARGET_PROPERTIES(${THIS})

install(
    TARGETS ${THIS}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES ${THIS_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
