#!/bin/bash

PACKAGE_DIR="./pkg"
UPDATABLE_FILES=()
UPDATABLE_DIRS=()
INSTALLABLE_FILES=()

check_if_exists(){
    PACKAGE_FILES=(./*)
    for PACKAGE_FILE in "${PACKAGE_FILES[@]}"; 
    do 
        if [ -f ${USER_PWD}/$PACKAGE_FILE ];
        then
            echo "File found:         ${USER_PWD}/$PACKAGE_FILE"
            UPDATABLE_FILES+=($PACKAGE_FILE) 
        elif [ -d ${USER_PWD}/$PACKAGE_FILE ]; 
        then 
            echo "Directory found:    ${USER_PWD}/$PACKAGE_FILE"
            UPDATABLE_DIRS+=($PACKAGE_FILE)
        else 
            echo "Not found:          ${USER_PWD}/$PACKAGE_FILE"
            INSTALLABLE_FILES+=($PACKAGE_FILE)
        fi
    done
}

#Arg1: Reference_File
#Arg2: Checked_file
get_file_diff(){
    REFERENCE_FILE=$1
    CHECKED_FILE=$2
    
    if [ -f $CHECKED_FILE ]; 
    then 
        diff -q $REFERENCE_FILE $CHECKED_FILE
    else 
        echo "$CHECKED_FILE is not a file! Aborting diff." 
    fi
}

#Arg1: File that contains marked lines
remove_marked_lines() {
    FILE=$1
    REGEX="\(#@\+[\s\S]*#@-.*\)"
    START_REGEX="\(#@\+[\s\S]*\)"
    END_REGEX="\(#@-.*\)"

    sed "/$START_REGEX/,/$END_REGEX/{//!d}" $FILE > ${FILE}_changed
    sed -i "/$START_REGEX/d" ${FILE}_changed
    sed -i "/$END_REGEX/d" ${FILE}_changed
}

check_updatable_files() {
    TMP_LIST=()
    for FILE in "${UPDATABLE_FILES[@]}"; 
    do
        FILE_DIF=$(get_file_diff ${USER_PWD}/$FILE ./$FILE) 
        if [ -z "$FILE_DIF" ];
        then 
            echo "$FILE has no applicable udpates!" 
        else 
            echo "$FILE is updatable! Adding to updatable file list"
            TMP_LIST+=$FILE
        fi
    done

    UPDATABLE_FILES=$TMP_LIST
}

# Arg1: File path 
# Arg2: Message
add_stub_cmake_lists_file(){
    file_path=$1
    msg=$2
    
    touch ${file_path}/CMakeLists.txt
    
    echo "cmake_minimum_required(VERSION 3.6) " > ${file_path}/CMakeLists.txt
    echo -en '\n' >> ${file_path}/CMakeLists.txt
    echo $msg >> ${file_path}/CMakeLists.txt
}

add_stub_sources(){
    mkdir -p ${USER_PWD}/sources/Example 
    
    add_stub_cmake_lists_file "${USER_PWD}/sources" "add_subdirectory(Example)" 
    add_stub_cmake_lists_file "${USER_PWD}/sources/Example" "message(WARNING \"Add some source directories and update ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt and ${USER_PWD}/sources/CMakeLists.txt files accordingly!\")"
    
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "set(THIS EXAMPLE)" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "list(APPEND \${THIS}_PUBLIC_INCLUDES_DIRS)" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "INCLUDE_DIRS_AND_FILES(\${THIS}_PUBLIC_INCLUDES_DIRS PUBLIC_INCLUDES)" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "PRINT_MODULE_FILES(\${THIS} \"\${sources_list}\" \"\${PUBLIC_INCLUDES}\")" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "add_library(\${THIS} INTERFACE)" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "target_include_directories(\${THIS} INTERFACE" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "                           $<INSTALL_INTERFACE:\${INCLUDES_DEST}>" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "                           $<INSTALL_INTERFACE:\${LIBS_DEST}>)" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "install(" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    TARGETS \${THIS} " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    EXPORT \${PROJECT_NAME} " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    ARCHIVE DESTINATION \${LIBS_DEST} " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    LIBRARY DESTINATION \${LIBS_DEST} " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    COMPONENT libraries " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo ")" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo -en '\n' >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "install(" >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    FILES \${includes_list} " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    DESTINATION \${INCLUDES_DEST} " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo "    COMPONENT headers " >> ${USER_PWD}/sources/Example/CMakeLists.txt
    echo ")" >> ${USER_PWD}/sources/Example/CMakeLists.txt
}

create_test_runner(){
    touch ${USER_PWD}/tests/unit_tests/test_runner.cpp
    echo "#include \"gtest/gtest.h\"" >> ${USER_PWD}/tests/unit_tests/test_runner.cpp
    echo -en '\n' >> ${USER_PWD}/tests/unit_tests/test_runner.cpp
    echo "int main(int argc, char **argv) {" >> ${USER_PWD}/tests/unit_tests/test_runner.cpp
    echo "  testing::InitGoogleTest(&argc, argv);" >> ${USER_PWD}/tests/unit_tests/test_runner.cpp
    echo "  return RUN_ALL_TESTS();" >> ${USER_PWD}/tests/unit_tests/test_runner.cpp
    echo "}" >> ${USER_PWD}/tests/unit_tests/test_runner.cpp
}

create_test_cmake(){
    add_stub_cmake_lists_file "${USER_PWD}/tests/unit_tests/" "message(WARNING \"Implement some tests and update ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt file accordingly!\")" 
echo "set(THIS Test_Runner)

file(GLOB Test_Suite \"\${CMAKE_CURRENT_LIST_DIR}/*.cpp\")
file(GLOB Test_Cases \"\${CMAKE_CURRENT_LIST_DIR}/*.hpp\")

INCLUDE_DIRS_AND_FILES(\"\${PUBLIC_INCLUDES_DIRS}\" PUBLIC_INCLUDES)

add_executable(\${THIS} \"\${Test_Suite}\" \"\${Test_Cases}\" \"\${PUBLIC_INCLUDES}\")

message(WARNING \"Add links to your code for test runner!\")

target_link_libraries(\${THIS} PUBLIC
    gtest_main
    gmock_main
)

add_test(
    NAME \${THIS}
    COMMAND \${THIS}
)

if(CLANG_TIDY_EXE)               
    set_target_properties(\${THIS} PROPERTIES
                                  CXX_CLANG_TIDY \"\${CMAKE_CXX_CLANG_TIDY}\"
)
endif()    

PRINT_TARGET_PROPERTIES(\${THIS})

if(WIN32)
    add_custom_command(TARGET \${THIS} POST_BUILD 
                        COMMAND \${CMAKE_COMMAND} -E copy_directory 
                            \"\${CMAKE_BINARY_DIR}/bin/\"
                            \$<TARGET_FILE_DIR:\${THIS}> 
                        COMMENT \"Copying shared library files from \${CMAKE_BINARY_DIR}/bin directory to \${CMAKE_CURRENT_BINARY_DIR} directory.\"
                            VERBATIM
    )
endif()"
}

add_stub_tests(){
    mkdir -p ${USER_PWD}/tests/unit_tests/ 
    create_test_cmake
    create_test_runner
}

add_stub_includes(){
    mkdir -p ${USER_PWD}/includes 
    
    touch ${USER_PWD}/includes/YOUR_HEADER_FILES_SHOULD_GO_HERE
}

do_install() {
    
    echo "Installing files into:"
    echo "${USER_PWD}/"
    cp -rf ${PACKAGE_DIR}/* ${USER_PWD}/
    cp -rf ${PACKAGE_DIR}/.* ${USER_PWD}/
    
    add_stub_sources
    add_stub_tests
    add_stub_includes
}

do_update(){ 
    check_updatable_files
    
    if [ ${#INSTALLABLE_FILES[@]} -ne 0 ]; 
    then 
        echo "Installing new files: ${INSTALLABLE_FILES[@]}" 
        for FILE in "${INSTALLABLE_FILES[@]}"; 
        do 
            rsync -rtv ${FILE} ${USER_PWD}/
        done
    elif [ ${#UPDATABLE_FILES[@]} -ne 0 ]; 
    then
        echo "Updating files: ${UPDATABLE_FILES[@]}"
        #update_files
    elif [ ${#UPDATABLE_DIRS[@]} -ne 0 ];
    then
        echo "Updating directories: ${UPDATABLE_DIRS[@]}"
        #update_dirs
    else 
        echo "Nothing to update! Aborting..."
        exit 1
    fi
}

do_cleanup(){
    echo "Cleaning up archives" 
    rm -rf ${USER_PWD}/Project_Template_Files*
    exit 0
}

# =================== MAIN ENTRY POINT ====================

check_if_exists
echo "This package contains:" 
ls -la $PACKAGE_DIR
echo "Installable: ${INSTALLABLE_FILES[@]}"
echo "Updatable: ${UPDATABLE_FILES[@]}"

if [ ${#INSTALLABLE_FILES[@]} -ne 0 ] && [ ${#UPDATABLE_FILES[@]} -eq 0 ]; 
then 
    echo "Deteced a clean project setup. Installing from scratch!"
    do_install
elif [ ${#UPDATABLE_FILES[@]} -ne 0 ];
then
    echo "Detected previous setup. Trying to update!"
    do_update
fi

do_cleanup
