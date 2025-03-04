cmake_minimum_required(VERSION 3.6) 

set(THIS Runner)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

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

if(WIN32)
    add_custom_command(TARGET ${THIS} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                        -t $<TARGET_FILE_DIR:${THIS}>
                        $<TARGET_RUNTIME_DLLS:${THIS}>
                        COMMAND_EXPAND_LISTS
    )
    # Workaround to get conan .dll files into build
    foreach(directory ${CONAN_RUNTIME_LIB_DIRS})
        file(GLOB dir_dependencies "${directory}/*.dll")
        if(dir_dependencies)
            list(APPEND dependencies "${dir_dependencies}")
        endif()
    endforeach()
    add_custom_command(TARGET ${THIS} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                        -t $<TARGET_FILE_DIR:${THIS}>
                        ${dependencies}
                        COMMAND_EXPAND_LISTS
    )
endif()
