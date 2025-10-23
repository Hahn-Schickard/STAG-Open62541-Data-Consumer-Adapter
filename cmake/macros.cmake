cmake_minimum_required(VERSION 3.10)

FUNCTION(PRINT_FILES file_list)
    if(VERBOSE_FILE_INCLUSION)
        if(NOT file_list)
            message(STATUS "| VERBOSE_OUTPUT |     no files found")
        else()
            foreach(thing ${file_list})
                message(STATUS "| VERBOSE_OUTPUT |   |-- ${thing}")
            endforeach()
        endif()
    endif()
ENDFUNCTION()

FUNCTION(PRINT_TARGET_PROPERTIES target_name)
    if(VERBOSE_FILE_INCLUSION)
        message(STATUS "| VERBOSE_OUTPUT | ${target_name} target properties are::")

        get_target_property(include_dirs ${target_name} INCLUDE_DIRECTORIES)
        if(include_dirs)
            message(STATUS "| VERBOSE_OUTPUT | ==Include Directories==")
            PRINT_FILES("${include_dirs}")
        endif()

        get_target_property(sources_list ${target_name} SOURCES)
        if(sources_list)
            message(STATUS "| VERBOSE_OUTPUT | =====Source Files=====")
            PRINT_FILES("${sources_list}")
        endif()

        get_target_property(interface_sources_list ${target_name} INTERFACE_SOURCES)
        if(interface_sources_list)
            message(STATUS "| VERBOSE_OUTPUT | =====Interface Files=====")
            PRINT_FILES("${interface_sources_list}")
        endif()

        get_target_property(linked_libs ${target_name} LINK_LIBRARIES)
        if(linked_libs)
            message(STATUS "| VERBOSE_OUTPUT | =====Linked Libraries=====")
            PRINT_FILES("${linked_libs}")
        endif()
    endif()
ENDFUNCTION()

FUNCTION(INCLUDE_DIRS directory_list)
    foreach(directory ${directory_list})
        include_directories(${directory})
    endforeach()
ENDFUNCTION()

FUNCTION(INCLUDE_DIRS_AND_FILES directory_list included_file_list)
    INCLUDE_DIRS("${directory_list}")
    foreach(directory ${directory_list})
        file(GLOB tmp_file_list ${directory}/*.hpp)
        list(APPEND included_file_list ${tmp_file_list})
    endforeach()
    set(${included_file_list} ${${included_file_list}} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(IMPORT_TARGET_DLLS target)
    get_target_property(target_type ${target} TYPE)
    if (target_type STREQUAL "EXECUTABLE")
        if(WIN32)
        message(STATUS "Importing ${target} .dll files")
        add_custom_command(TARGET ${target} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy
                            -t $<TARGET_FILE_DIR:${target}>
                            $<TARGET_RUNTIME_DLLS:${target}>
                            COMMAND_EXPAND_LISTS
        )
        # Workaround to get conan .dll files into build
        foreach(directory ${CONAN_RUNTIME_LIB_DIRS})
            file(GLOB dir_dependencies "${directory}/*.dll")
            if(dir_dependencies)
                list(APPEND dependencies "${dir_dependencies}")
            endif()
        endforeach()
        add_custom_command(TARGET ${target} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy
                            -t $<TARGET_FILE_DIR:${target}>
                            ${dependencies}
                            COMMAND_EXPAND_LISTS
        )
        else()
            message(STATUS "Not a Windows OS, skipping .dll file import for ${target} target")
        endif()
    else()
        message(STATUS "Target ${target} is not an executable")
    endif ()
ENDFUNCTION()

FUNCTION(INSTALL_TARGET target)
    install(
        TARGETS ${target}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME
            DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
ENDFUNCTION()

FUNCTION(INSTALL_HEADERS)
    cmake_parse_arguments(
        ARG # prefix
        "" # flags
        "DESTINATION" # single value args
        "HEADERS" # multi value args
        ${ARGN} # parsed args
    )

    if(NOT ARG_HEADERS)
        message(FATAL_ERROR "You must provide a list of header files to install")
    endif(NOT ARG_HEADERS)

    set(BASE_HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

    if(ARG_DESTINATION)
        set(HEADER_INSTALL_DIR ${BASE_HEADER_INSTALL_DIR}/${ARG_DESTINATION})
    else()
        set(HEADER_INSTALL_DIR ${BASE_HEADER_INSTALL_DIR})
    endif(ARG_DESTINATION)

    install(
        FILES
            ${ARG_HEADERS}
        DESTINATION
            ${HEADER_INSTALL_DIR}
    )
ENDFUNCTION()
