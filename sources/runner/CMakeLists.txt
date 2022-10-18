cmake_minimum_required(VERSION 3.6) 

set(THIS Runner)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

find_package(GTest REQUIRED)

add_executable(${THIS})              

target_sources(${THIS} PRIVATE 
                       ${sources_list}
)

target_link_libraries(${THIS} PRIVATE
                              OPCUA_Adapter
                              GTest::GTest
)

target_compile_features(${THIS} PUBLIC cxx_std_17)

add_custom_command(TARGET ${THIS} POST_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${PROJECT_SOURCE_DIR}/config
                        $<TARGET_FILE_DIR:${THIS}>/config 
                    COMMENT "Copying configuration files."
                        VERBATIM
)
