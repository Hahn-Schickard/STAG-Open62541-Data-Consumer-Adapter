#!/bin/bash

BUILD_DIR=$1 
TYPE=$2
INSTALLER_SCRIPT="./.template_installer.sh"

RELEASE_TAG=""
VERSION=""
MAJOR_VER="0"
MINOR_VER="0"
PATCH_VER="0"

get_version() {
    Search_Regex="^(Release_)([0-9])+"
    Superfluous_Regex="^(Release_)"
    
    tmp=$(sed "s/${Search_Regex}//p" <<< $RELEASE_TAG)
    VERSION=$(sed -r "s/${Superfluous_Regex}//" <<< $tmp)
    MAJOR_VER=$(sed -n 's/^\([0-9]*\).*/\1/p' <<< $VERSION)
    MINOR_VER=$(sed -n "s/^\(${MAJOR_VER}.\)//p" <<< $VERSION)
    MINOR_VER=$(sed -n 's/^\([0-9]*\).*/\1/p' <<< $MINOR_VER)
    PATCH_VER=$(sed -n "s/^\(${MAJOR_VER}.${MINOR_VER}.\)//p" <<< $VERSION)
    PATCH_VER=$(sed -n 's/^\([0-9]*\).*/\1/p' <<< $PATCH_VER)
    
    if [ "0.0.0" == "${MAJOR_VER}.${MINOR_VER}.${PATCH_VER}" ];
    then 
        echo "Failed to set version number from tag info! Aborting..."
        exit 1
    else
        echo "Release version is set to: ${MAJOR_VER}.${MINOR_VER}.${PATCH_VER}"
    fi
}


create_dir() {
    if [ -d "${BUILD_DIR}/package" ];
    then 
        rm -rf ${BUILD_DIR}/package
    fi

    mkdir -p ${BUILD_DIR}/package
}

#Arg1: file to be copied
copy_file_into_package(){
    FILE=$1
    
    echo "Coppying $FILE into ${BUILD_DIR}/package/"
    cp -r $FILE ${BUILD_DIR}/package/${FILE}
}

remvoe_template_specific_jobs() {
    echo "Removing template specific jobs from ${BUILD_DIR}/.gitlab-ci.yml"
    CI_FILE="${BUILD_DIR}/package/.gitlab-ci.yml"
    REGEX="\(#@\+[\s\S]*#@-.*\)"
    START_REGEX="\(#@\+[\s\S]*\)"
    END_REGEX="\(#@-.*\)"

    sed -i "/$START_REGEX/,/$END_REGEX/{//!d}" $CI_FILE
    sed -i "/$START_REGEX/d" $CI_FILE
    sed -i "/$END_REGEX/d" $CI_FILE
}

copy_package_files(){
    copy_file_into_package cmake
    mkdir -p ${BUILD_DIR}/package/docs/code_documentation
    copy_file_into_package docs/code_documentation/doc_main_page.md
    copy_file_into_package docs/code_documentation/doxygen_special_command_cheatsheet.md
    copy_file_into_package docs/code_documentation/stylesheet.css 
    copy_file_into_package docs/code_documentation/vendor-logo.png
    copy_file_into_package docs/code_documentation/header.html 
    copy_file_into_package docs/code_documentation/footer.html 
    copy_file_into_package .gitignore 
    copy_file_into_package .gitlab-ci.yml
    remvoe_template_specific_jobs
    copy_file_into_package CMakeLists.txt
    copy_file_into_package CONTRIBUTING.md
    copy_file_into_package GoogleTestDependency.txt
    copy_file_into_package ci-variables.yml 
    mkdir -p ${BUILD_DIR}/package/.gitlab/issue_templates 
    copy_file_into_package .gitlab/issue_templates/Bug.md
    copy_file_into_package .gitlab/issue_templates/Feature.md 
    copy_file_into_package .gitlab/issue_templates/Task.md
    copy_file_into_package $INSTALLER_SCRIPT
    cp -r utility/ ${BUILD_DIR}/package/utility
}

#Arg1: Compresion tool
check_compression_tool(){
    TOOL=$1
    CMD=$($TOOL --version)
    RETURN_CODE=$?
    
    if [ $RETURN_CODE -ne 0 ];
    then 
        echo "Compression tool: $TOOL can not be found. Aborting..." 
        exit 1
    fi
}

test_makeself(){
    CMD=$(makeself.sh --version)
    RETURN_CODE=$?
    
    if [ $RETURN_CODE -ne 0 ];
    then 
        echo "makeself tool can not be found. Please install it by running the following commands: "
        echo ""
        echo "wget https://github.com/megastep/makeself/releases/download/release-2.4.0/makeself-2.4.0.run"
        echo "chmod 755 makeself-2.4.0.run"
        echo "./makeself-2.4.0.run"
        echo "cd makeself-2.4.0"
        echo "sudo cp *.sh /usr/bin"
        echo "cd .."
        echo "echo \"alias makeslef=\"makeself.sh\"\" >> ~/.bashrc"
        echo "rm -rf makeself-2.4.0.run makeself-2.4.0"
        exit 1
    fi
}

#Arg1: Compresion tool
#Arg2: Package name
create_self_extracting_archive(){
    COMPRESSER=$1 
    PACKAGE_NAME=$2
    
    if [ -z "$COMPRESSER" ];
    then 
        echo "Compression tool has not been specified! Aborting..." 
        exit 1
    elif [ -z "$PACKAGE_NAME" ]; 
    then
        echo "Package name has not been specified! Aborting..." 
        exit 1
    fi    
    
    if [[ "zip" != "$COMPRESSER" ]];
    then
        test_makeself
        check_compression_tool $COMPRESSER
        echo "Archiving files from $(pwd)"
        mkdir ${BUILD_DIR}/package/pkg 
        mv -v ${BUILD_DIR}/package/* ${BUILD_DIR}/package/pkg/
        mv -v ${BUILD_DIR}/package/.* ${BUILD_DIR}/package/pkg/
        mv ${BUILD_DIR}/package/pkg/${INSTALLER_SCRIPT} ${BUILD_DIR}/package/
        makeself.sh --${COMPRESSER} ${BUILD_DIR}/package ${PACKAGE_NAME}_${VERSION}.sh "${PACKAGE_NAME} v$VERSION self-extracting package." $INSTALLER_SCRIPT
    else
        original_dir=$(pwd)
        cd ${BUILD_DIR}/package
        echo "Archiving files from $(pwd)"
        zip -y -r -v -A ${PACKAGE_NAME}_${VERSION}.zip .
        mv ${PACKAGE_NAME}_${VERSION}.zip $original_dir
        cd $original_dir
    fi
    rm -rf ${BUILD_DIR}/package/*
    rm -rf ${BUILD_DIR}/package/.*
    mv ${PACKAGE_NAME}_${VERSION}.* ${BUILD_DIR}/package/
}


package_for_linux() {
    create_self_extracting_archive "gzip" "Project_Template_Files_Linux_Installer" 
}

package_for_windows() {
    create_self_extracting_archive "zip" "Project_Template_Files_Windows_Installer" 
}

do_package() {
    RELEASE_TAG=$(git describe --exact-match --abbrev=0)
    git_cmd_status=$?

    if [ $git_cmd_status -eq 0 ]; 
    then
        echo "Packaging Release files"
        create_dir
        get_version
        copy_package_files
        if [ "windows" == "${TYPE,,}" ];
        then
            package_for_windows
        elif [ "linux" == "${TYPE,,}" ];
        then
            package_for_linux
        else
            echo "Unrecognized pacakging environment. Aborting..."
            exit 1
        fi
    else 
        echo "This commit is not tagged. Aborting realease."
        exit 1
    fi
}

do_package
