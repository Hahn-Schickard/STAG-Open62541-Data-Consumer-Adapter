cmake_minimum_required(VERSION 3.6)

set (THIS Data_Value_Identifier)

file(GLOB sources_list "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB includes_list "${PROJECT_SOURCE_DIR}/includes/utilities/*.hpp")

PRINT_MODULE_FILES(${THIS} "${sources_list}" "${includes_list}")

include_directories(${PROJECT_SOURCE_DIR}/includes/utilities)

add_library(${THIS} SHARED
            ${sources_list}
            ${includes_list})

if (WIN32)
    target_link_libraries(${THIS} PUBLIC
                                  Node_Information
                                  C_String_Formater)
elseif (UNIX)
    target_link_libraries(${THIS} PUBLIC 
                                  gcov
                                  Node_Information
                                  C_String_Formater)
else ()
    target_link_libraries(${THIS} PUBLIC
                                  Node_Information
                                  C_String_Formater)
    message(WARNING "Please configure your linking for your operating system for target ${THIS}")
endif ()
               
target_include_directories(${THIS} INTERFACE
                           $<BUILD_INTERFACE:${utilities_includes_dir}>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           $<INSTALL_INTERFACE:${INCLUDES_DEST}>
                           $<INSTALL_INTERFACE:${LIBS_DEST}>)
if(CLANG_TIDY_EXE)               
    set_target_properties(
                        ${THIS} PROPERTIES
                        CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY}
                        )
endif()    

add_custom_command(TARGET ${THIS} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E make_directory 
            ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_FILE:${THIS}> 
            "${CMAKE_BINARY_DIR}/bin"
        COMMENT "Copying shared library files from ${CMAKE_CURRENT_BINARY_DIR} directory to ${CMAKE_BINARY_DIR}/bin directory."
            VERBATIM)         

install(
    TARGETS ${THIS} 
    EXPORT ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LIBS_DEST}
    LIBRARY DESTINATION ${LIBS_DEST}
    COMPONENT libraries
)

install(
    FILES ${includes_list}
    DESTINATION ${INCLUDES_DEST}
    COMPONENT headers
)
