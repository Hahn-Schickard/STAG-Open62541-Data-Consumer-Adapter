#!/bin/bash

CI_REPOSITORY_URL=$1
MANUAL_TRIGER=$2
RELEASE_TYPE=$3
COMMIT_MESSAGE=$(git log -1 --pretty=%B)
REGEX="^Signed-off-by"
COMMIT_MESSAGE=$(sed "/${REGEX}/d" <<< $COMMIT_MESSAGE)
LAST_TAG=$(git describe --abbrev=0)
MAJOR_RELEASE_TYPE="Major Release"
MINOR_RELEASE_TYPE="Minor Release" 
PATCH_RELEASE_TYPE="Patch Release"
LAST_VERSION=""
LAST_MAJOR_VER=""
LAST_MINOR_VER=""
LAST_PATCH_VER=""
MAJOR_VER="0"
MINOR_VER="0"
PATCH_VER="0"

do_incrament(){
    if [ "${MANUAL_TRIGER,,}" == "yes" ];
    then 
        manual_trigger_incrament
    else 
        automatic_trigger_incrament
    fi
    add_new_git_tag
}

automatic_trigger_incrament() {
    check_commit_message
    update_version_info
}

manual_trigger_incrament(){
    if [ "${RELEASE_TYPE^^}" == "MAJOR" ];
    then 
        incrament_major_version
    elif [ "${RELEASE_TYPE^^}" == "MINOR" ];
    then
        incrament_minor_version
    elif [ "${RELEASE_TYPE^^}" == "PATCH" ];
    then 
        incrament_patch_version
    else 
        echo "Realease type is unknown. Please specify what is the release type MAJOR|MINOR|PATCH"
        exit 1
    fi
}

check_if_first_tag() {
    git describe --abbrev=0
    git_cmd_status=$?
    
    if [ $git_cmd_status -eq 128 ]; 
    then 
        echo "This commit is the first tag!"
        LAST_MAJOR_VER="0"
        LAST_MINOR_VER="0"
        LAST_PATCH_VER="0"
        do_incrament
    else 
        check_if_taged
        get_previous_version_info
        do_incrament
    fi   
}

check_if_taged() {
    echo "running git describe get the tag and commit information"
    git describe --exact-match --abbrev=0
    git_cmd_status=$?
    
    if [ $git_cmd_status -eq 0 ]; 
    then
        echo "This commit is already tagged"
        exit 0
    else 
        echo "This commit is not tagged, trying to add versioning number"
    fi
}

get_previous_version_info() {
    Search_Regex="^(Release_)([0-9])+"
    Superfluous_Regex="^(Release_)"
    
    tmp=$(sed "s/${Search_Regex}//p" <<< $LAST_TAG)
    LAST_VERSION=$(sed -r "s/${Superfluous_Regex}//" <<< $tmp)
    LAST_MAJOR_VER=$(sed -n 's/^\([0-9]*\).*/\1/p' <<< $LAST_VERSION)
    LAST_MINOR_VER=$(sed -n "s/^\(${LAST_MAJOR_VER}.\)//p" <<< $LAST_VERSION)
    LAST_MINOR_VER=$(sed -n 's/^\([0-9]*\).*/\1/p' <<< $LAST_MINOR_VER)
    LAST_PATCH_VER=$(sed -n "s/^\(${LAST_MAJOR_VER}.${LAST_MINOR_VER}.\)//p" <<< $LAST_VERSION)
    LAST_PATCH_VER=$(sed -n 's/^\([0-9]*\).*/\1/p' <<< $LAST_PATCH_VER)
    
    echo "Previous tag was: Release_${LAST_MAJOR_VER}.${LAST_MINOR_VER}.${LAST_PATCH_VER}"
}

incrament_major_version() {
    MAJOR_VER=$((LAST_MAJOR_VER + 1))
    MINOR_VER="0"
    PATCH_VER="0"
}

incrament_minor_version() {
    MAJOR_VER=$LAST_MAJOR_VER
    MINOR_VER=$((LAST_MINOR_VER + 1))
    PATCH_VER="0"
}

incrament_patch_version() {
    MAJOR_VER=$LAST_MAJOR_VER
    MINOR_VER=$LAST_MINOR_VER
    PATCH_VER=$((LAST_PATCH_VER + 1))
}

check_commit_message() {
    echo "Commit message: "
    echo "${COMMIT_MESSAGE}"
    if [[ ${COMMIT_MESSAGE,,} == *${MAJOR_RELEASE_TYPE,,}* ]];
    then 
        echo "Incramenting Major version number"
        incrament_major_version
    elif [[ ${COMMIT_MESSAGE,,} == *${MINOR_RELEASE_TYPE,,}* ]];
    then 
        echo "Incramenting Minor version number"
        incrament_minor_version
    elif [[ ${COMMIT_MESSAGE,,} == *${PATCH_RELEASE_TYPE,,}* ]];
    then 
        echo "Incramenting Patch version number"
        incrament_patch_version
    else 
        echo "Not a Release commit, not tagging."
        exit 0
    fi
}

update_version_info() {
    OLD_CI_MAJOR_VER=" RELEASE_MAJOR_VERSION:.*"
    OLD_CI_MINOR_VER=" RELEASE_MINOR_VERSION:.*"
    OLD_CI_PATCH_VER=" RELEASE_PATCH_VERSION:.*"
    NEW_CI_MAJOR_VER=" RELEASE_MAJOR_VERSION: \"${MAJOR_VER}\""
    NEW_CI_MINOR_VER=" RELEASE_MINOR_VERSION: \"${MINOR_VER}\""
    NEW_CI_PATCH_VER=" RELEASE_PATCH_VERSION: \"${PATCH_VER}\""
    
    sed -i -e "s/$OLD_CI_MAJOR_VER/$NEW_CI_MAJOR_VER/g" ci-variables.yml
    sed -i -e "s/$OLD_CI_MINOR_VER/$NEW_CI_MINOR_VER/g" ci-variables.yml
    sed -i -e "s/$OLD_CI_PATCH_VER/$NEW_CI_PATCH_VER/g" ci-variables.yml
    
    echo "Trying to set new tag as: Release_${MAJOR_VER}.${MINOR_VER}.${PATCH_VER}"
}

add_new_git_tag() {
    Search_Regex="\(@git.*\)"
    REPO_URL=$(sed -n "s/.*${Search_Regex}.*/\1/p" <<< $CI_REPOSITORY_URL)
    REPO_URL=$(sed '0,/\//s/\//:/' <<< $REPO_URL)

    git remote set-url --push origin "git${REPO_URL}"
    echo "Pushing tags to git${REPO_URL}"
    git tag -a Release_${MAJOR_VER}.${MINOR_VER}.${PATCH_VER} -m "v${MAJOR_VER}.${MINOR_VER}.${PATCH_VER} Release" -m "${COMMIT_MESSAGE}" -m "This release, has been trigered by a pipeline." -m "Released at: $(date +"%D %T")"
    git push origin Release_${MAJOR_VER}.${MINOR_VER}.${PATCH_VER} 
    exit 0
}

check_if_first_tag