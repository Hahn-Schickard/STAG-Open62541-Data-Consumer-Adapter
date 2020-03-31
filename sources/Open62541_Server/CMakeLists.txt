cmake_minimum_required(VERSION 3.6) 

set(THIS OPCUA_Server)

list(APPEND PUBLIC_INCLUDES_DIRS
            ${INCLUDES_DIR}/Open62541_Server
)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

INCLUDE_DIRS_AND_FILES("${THIS_INCLUDES_DIRS}" THIS_INCLUDES)

add_library(${THIS})

target_sources(${THIS} PRIVATE 
                       ${THIS_INCLUDES}
                       ${sources_list}
)

target_link_libraries(${THIS} PUBLIC
                              Information_Model::Information_Model
                              open62541::open62541
                              nlohmann_json::nlohmann_json
                              HaSLL::HaSLL   
)
                            
if (UNIX)
    target_link_libraries(${THIS} PRIVATE
                                  gcov
    )
endif()

target_include_directories(${THIS} PUBLIC
                           $<BUILD_INTERFACE:${INCLUDES_DIR}/Open62541_Server>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
)

target_compile_features(${THIS} PUBLIC cxx_std_17)

PRINT_TARGET_PROPERTIES(${THIS})

install(
    TARGETS ${THIS}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES ${THIS_INCLUDES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)