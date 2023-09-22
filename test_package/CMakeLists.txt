cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(Open62541_Data_Consumer_Adapter CONFIG REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example Open62541_Data_Consumer_Adapter::Open62541_Data_Consumer_Adapter)

add_custom_command(TARGET example POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${PROJECT_SOURCE_DIR}/../config
                        $<TARGET_FILE_DIR:example>/config
                    COMMENT "Copying configuration files."
                        VERBATIM
)
