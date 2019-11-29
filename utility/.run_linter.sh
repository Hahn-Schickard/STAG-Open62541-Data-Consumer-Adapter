#!/bin/bash

BUILD_DIR=$1
PROJECT_URL=$2		# CI_REPOSITORY_URL
PROJECT_ID=$3       # CI_PROJECT_ID
API_URL=$4          # CI_API_V4_URL
USER=$5             # GITLAB_USER_ID
BRANCH_ID=$6        # CI_COMMIT_REF_NAME
      
check_clang_tidy(){
    clang-tidy --help >/dev/null
    clang_tidy_status=$?
    if [[ "$clang_tidy_status" -ne 0 ]];
    then 
        echo "clang-tidy is not installed! Aborting"
        exit 1
    fi 
}

check_clang_format(){
    clang-format --help >/dev/null
    clang_format_status=$? 
    if [[ "$clang_format_status" -ne 0 ]];
    then 
        echo "clang-format is not installed! Aborting"
        exit 1
    fi 
}

check_build_dir(){
    if [ ! -d "$BUILD_DIR" ];
    then 
        echo "Specified directory: $BUILD_DIR does not exist. Aborting..."
        exit 1
    fi
}

setup_results_dir(){
    if [ ! -d linting_results ];
    then 
        rm -rf linting_results
    fi 
    mkdir linting_results
}

run_clang_format(){
    cwd="$(pwd)"
    cd ${BUILD_DIR} 
    make clangformat |& tee clangformat_output
    sed -n '/Formatting/p' clangformat_output > tmp.txt
    sed 's/^Formatting //' tmp.txt > formated_files.txt
    if [ ! -s formated_files.txt ];
    then
        rm formated_files.txt
    else
        push_to_git $PROJECT_URL $PROJECT_ID $API_URL $USER $BRANCH_ID
        status=$?
        if [ "$status" -ne 0 ];
        then
            echo "This script should be run by gitlab-runner!"
        fi
        mv formated_files.txt $cwd/linting_results/
    fi
    cd $cwd 
    mv clang-tidy-suggested-fixes.yml linting_results/
}

check_clang_tidy
check_clang_format
check_build_dir
setup_results_dir
run_clang_format