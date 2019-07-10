#!/bin/bash 
BUILD_DIR=$1
PROJECT_ID=$2
API_LINK=$3
RELEASE_TAG=""
VERSION=""
MAJOR_VER="0"
MINOR_VER="0"
PATCH_VER="0"

check_args() {
    if [ -z "$BUILD_DIR" ];
    then 
        echo "Build directory has not been provided! Aborting..." 
        exit 1
    elif [ -z "$PROJECT_ID" ];
    then
        echo "Project ID has not been provided! Aborting..."
        exit 1
    elif [ -z "$API_LINK" ]; 
    then
        echo "Project REST API has not been provided! Aborting..." 
        exit 1
    fi
}

upload_binaries() {
    
}

send_post_cmd() {

}

do_release() {
    check_args
    
    RELEASE_TAG=$(git describe --exact-match --abbrev=0)
    git_cmd_status=$?
    
    if [ $git_cmd_status -eq 0 ]; 
    then
        echo "Configuring a Realease"
        get_version
        check_build_dir
        create_build_config
    else 
        echo "This commit is not tagged. Aborting realease."
        exit 1
    fi
}

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

check_build_dir() {

    if [ -d "$BUILD_DIR" ];
    then
        echo "Cleaning up configuration environment. Removing $BUILD_DIR"
        rm -rf $BUILD_DIR
    fi
    
    echo "Setting up new configuration environment in $BUILD_DIR"
    mkdir -p $BUILD_DIR
}

create_build_config() {
    cd $BUILD_DIR 
    cmake -DCMAKE_BUILD_TYPE="Release" -DPACKAGE_MAJOR_VER=$MAJOR_VER -DPACKAGE_MINOR_VER=$MINOR_VER -DPACKAGE_PATCH_VER=$PATCH_VER ..
}

do_release
