cmake_minimum_required(VERSION 3.6) 

set(THIS Runner)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(${THIS})              

target_sources(${THIS} PRIVATE 
                       ${sources_list}
)

target_link_libraries(${THIS} PRIVATE
                              OPCUA_Adapter
)

target_compile_features(${THIS} PUBLIC cxx_std_17)