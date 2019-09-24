cmake_minimum_required(VERSION 3.6) 

# Pass your lists in quations, for example:
# PRINT_FILES("${file_list}")
MACRO(PRINT_FILES file_list)
    foreach(thing ${file_list})
        message(STATUS " |-- ${thing}")
    endforeach()
    message("----------------------------------------------------------------------------------------------------------------")
    message(" ")
ENDMACRO()

MACRO(PRINT_MODULE_FILES module_name sources_list includes_list)
    if(VERBOSE_FILE_INCLUSION)
        message(STATUS "${module_name} library files are:")
        message(STATUS "=====Source Files=====")
        PRINT_FILES("${sources_list}")
        message(STATUS "=====Include Files=====")
        PRINT_FILES("${includes_list}")
    endif()
ENDMACRO()

FUNCTION(INCLUDE_DIRS directory_list)
    foreach(directory IN LISTS ${directory_list})
        message(STATUS "Including directory: ${directory}")
        include_directories(${directory})
    endforeach()
ENDFUNCTION()

FUNCTION(INCLUDE_DIRS_AND_FILES directory_list included_file_list)
    INCLUDE_DIRS(${directory_list})
    foreach(directory IN LISTS ${directory_list})
        file(GLOB tmp_file_list ${directory}/*.hpp)
        list(APPEND included_file_list ${tmp_file_list})
    endforeach()
    set(${included_file_list} ${${included_file_list}} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(CREATE_PKG_CONFIG_FILE package_name config_file)
    string(TOLOWER ${package_name} config_file_name)
    string(CONCAT config_file_name ${config_file_name} "-config.cmake")
    if(EXISTS ${PROJECT_SOURCE_DIR}/${config_file_name})
        message(STATUS "Pacakge configuration file: ${PROJECT_SOURCE_DIR}/${config_file_name} already exists")
    else()
        message(STATUS "Creating package configuration file: ${PROJECT_SOURCE_DIR}/${config_file_name}")
        file(WRITE ${PROJECT_SOURCE_DIR}/${config_file_name}
            "get_filename_component(SELF_DIR \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
include(\${SELF_DIR}/lib/${package_name}.cmake)")
    endif()
    set(${config_file} ${config_file_name} PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(APPND_ONCE_TO_FILE file_name msg)
    file(READ ${file_name} file_content)
    string(FIND ${file_content} ${msg} content_found) 
    if(${content_found} EQUAL "-1")
        file(APPEND ${file_name} "
${msg}")
    else()
        message(STATUS "Message: 
        ${msg}
already exists in file ${file_name}")
    endif()
ENDFUNCTION()

FUNCTION(PROPOGATE_DEPENDENCY_PACKAGE this_projcet package_name)
    CREATE_PKG_CONFIG_FILE(${this_projcet} pkg_config_file)
    get_filename_component(dependency_package ${package_name} NAME)
    string(REGEX MATCH "(-+[0-9][^ ]*)" deletion_mask ${dependency_package})
    string(REPLACE ${deletion_mask} "" dependency_config_file ${dependency_package})
    string(TOLOWER ${dependency_config_file} dependency_config_file)
    message(STATUS "Appending to ${PROJECT_SOURCE_DIR}/${pkg_config_file} dependency: 
    ${dependency_package}
with configuration file: 
    ${dependency_config_file}-config.cmake")
    APPND_ONCE_TO_FILE("${PROJECT_SOURCE_DIR}/${pkg_config_file}"
        "include(\${CMAKE_CURRENT_LIST_DIR}/modules/${dependency_package}/${dependency_config_file}-config.cmake)")
ENDFUNCTION()
