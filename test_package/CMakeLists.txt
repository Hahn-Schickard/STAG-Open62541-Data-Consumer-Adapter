cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(Data_Consumer_Adapter_pen62541 CONFIG REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example Data_Consumer_Adapter_pen62541::Data_Consumer_Adapter_pen62541)

add_custom_command(TARGET example POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${PROJECT_SOURCE_DIR}/../config
                        $<TARGET_FILE_DIR:example>/config
                    COMMENT "Copying configuration files."
                        VERBATIM
)
