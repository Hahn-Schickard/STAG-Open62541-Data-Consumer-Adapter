#!/bin/bash

ROOT_CMAKE_FILE="./CMakeLists.txt"
DOXYFILE="./Doxyfile"

PROJECT_NAME="" 
PROJECT_LOGO=""
PROJECT_BRIEF="" 
PROJECT_MAINTAINERS=""
PACKAGE_VENDOR=""
CONTACT_INFO=""

usage(){
    echo "-n | --name           - Specifies projects name. DO NOT USE WHITESPACES! Use _ charachters instead!"
    echo "-b | --brief          - Specifies project description in a brief summary."
    echo "-m | --maintainers    - Specifies project authros/maintainers."
    echo "-v | --vendor         - Specifies package vendor."
    echo "-c | --contact        - Specifies maintainer email for project users."
    echo "-l | --logo           - Specifies project logo, if any. "
    echo "-h | --help           - Print this message"
    echo ""
    echo "=============== EXAMPLE CALL ==============="
    echo "./configure_project.sh  -n \"Template_Project\" -b \"Short project description\" -m \"Dovydas Girdvainis, Tasso Kaup, Gerhard Marki\" -v \"Hahn-Schickard\" -c \"info@hahn-schickard.de\" -l \"./docs/code_documentation/hs_logo.png\" "
}

pritify_project_name() {
    WHITESPACE=" "
    PROJECT_NAME=$(sed -n "s/_/ /p" <<< $PROJECT_NAME)
}

set_project_name() {
    echo "Setting project name to ${PROJECT_NAME} in $ROOT_CMAKE_FILE"
    sed -i "s/\(Project_Template_Example_Code\)/${PROJECT_NAME}/g" $ROOT_CMAKE_FILE
    
    pritify_project_name
    echo "Setting project name to ${PROJECT_NAME} in $DOXYFILE"
    sed -i "s/\(Project Template\)/${PROJECT_NAME}/g" $DOXYFILE

}

set_project_logo() {
    if [ -f $PROJECT_LOGO ];
    then
        echo "Setting project logo to ${PROJECT_LOGO} in $DOXYFILE" 
        sed -i "s#\(PROJECT_LOGO=\)#\(PROJECT_LOGO=${PROJECT_LOGO}\)#g" $DOXYFILE
    else 
        echo "Logo ${PROJECT_LOGO} does not exist! Aborting..."
        exit 1
    fi
}

set_project_brief() {
    echo "Setting project descrtiption to ${PROJECT_BRIEF} in $ROOT_CMAKE_FILE"
    sed -i "s/\(This is an example project with template files.\)/${PROJECT_BRIEF}/g" $ROOT_CMAKE_FILE
    
    echo "Setting project descrtiption to ${PROJECT_BRIEF} in $DOXYFILE"
    sed -i "s/\(PROJECT_BRIEF=\)/\(PROJECT_BRIEF=${PROJECT_BRIEF})/g" $DOXYFILE
    
    touch README.md
    echo "# ${PROJECT_NAME}" > README.md
    echo -en '\n' >> README.md
    
    if [ -n "$PROJECT_LOGO" ] && [ -f $PROJECT_LOGO ];
    then
        echo "![logo](${PROJECT_LOGO})" >> README.md
        echo -en '\n' >> README.md
    fi
    
    echo "## Brief description" >> README.md
    echo -en '\n' >> README.md
    echo ${PROJECT_BRIEF} >> README.md
}

set_project_maintainers() {
    echo "Setting project maintainers to ${PROJECT_MAINTAINERS} in $ROOT_CMAKE_FILE"
    sed -i "s/\(Dovydas Girdvainis\)/${PROJECT_MAINTAINERS}/g" $ROOT_CMAKE_FILE
}

set_package_vendor() {
    echo "Setting package vendor to ${PACKAGE_VENDOR} in $ROOT_CMAKE_FILE"
    sed -i "s/\(Hahn-Schickard\)/${PACKAGE_VENDOR}/g" $ROOT_CMAKE_FILE
}

set_contact_information() {
    echo "Setting contact info to ${CONTACT_INFO} in $ROOT_CMAKE_FILE"
    sed -i "s/\(dovydas.girdvainis@hahn-schickard.de\)/${CONTACT_INFO}/g" $ROOT_CMAKE_FILE
}

check_args() {
    if [ "$PROJECT_NAME" != "" ]; 
    then 
        set_project_name
    else 
        echo "Project name has not been specified! Aborting..." 
        usage
        exit 1
    fi 
    
    if [ "$PROJECT_LOGO" != "" ]; 
    then 
        set_project_logo
    fi
    
    if [ "$PROJECT_BRIEF" != "" ]; 
    then 
        set_project_brief
    else
        echo "Using placeholder project description! "
        PROJECT_BRIEF="Placeholder project description." 
        set_project_brief
    fi 
    
    if [ "$PROJECT_MAINTAINERS" != "" ]; 
    then 
        set_project_maintainers
    else
        echo "Project has no maintainers specified! Aborting..."
        usage
        exit 1
    fi 
    
    if [ "$PACKAGE_VENDOR" != "" ]; 
    then 
        set_package_vendor 
    else 
        echo "Package vendor has not been specified! Aborting..." 
        usage
        exit 1
    fi
    
    if [ "$CONTACT_INFO" != "" ];
    then 
        set_contact_information
    else
        echo "Contact email has not been specified! Aborting..." 
        usage
        exit 1
    fi     
}

while [ -n "$1" ];
do
	case $1 in
		-n | --name)
			shift
			if [ -n "$1" ];
			then
				PROJECT_NAME=${1}
			else
                echo "Argument -n passed, but not set. Please specify proejct name. Aborting..."
				usage
				exit 1
			fi
			;;
        -b | --brief)
			shift
			if [ -n "$1" ];
			then
				PROJECT_BRIEF=${1}
			else
                echo "Argument -b passed, but not set. Plase specify brief project description. Aborting..."
				usage
				exit 1
			fi
			;;
        -m | --maintainers)
			shift
			if [ -n "$1" ];
			then
				PROJECT_MAINTAINERS=${1}
			else
                echo "Argument -m passed, but not set. Please specify maintainers. Aborting..."
				usage
				exit 1
			fi
			;;
        -v | --vendor)
			shift
			if [ -n "$1" ];
			then
				PACKAGE_VENDOR=${1}
			else
                echo "Argument -v passed, but not set. Please specify vendor name. Aborting..."
				usage
				exit 1
			fi
			;;
        -c | --contact)
			shift
			if [ -n "$1" ];
			then
				CONTACT_INFO=${1}
			else
                echo "Argument -c passed, but not set. Please specify contact email. Aborting..."
				usage
				exit 1
			fi
			;;
        -l | --logo)
			shift
			if [ -n "$1" ];
			then
				PROJECT_LOGO=${1}
			else
                echo "Argument -l passed, but not set. Please specify path to projects logo. Aborting..."
				usage
				exit 1
			fi
			;;
		-h | --help)
		        usage
		        exit 0
		        ;;
		*)
			usage
			exit 0
			;;
	esac
	shift
done

check_args
