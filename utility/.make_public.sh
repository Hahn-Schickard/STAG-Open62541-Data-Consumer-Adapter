#!/bin/bash

PROJECT_NAME=""
PROJECT_URL=""
FILE_LIST=()

usage(){
    echo "-n | --name           - Specifies projects name."
    echo "-l | --link           - Specifies projects URL."
    echo ""
    echo "=============== EXAMPLE CALL ==============="
    echo "./.make_public.sh  -n \"Template_Project\" -l https://some.project-url.com"
}

setup_env(){
    if [ -d public ]; 
    then 
        rm -rf public/
        mkdir public/
    else 
        mkdir public/
    fi 
}

setup_amalgam_index(){
    cp -v utility/.amalgam_index.html public/index.html
    cp -v utility/.amalgam.css public/amalgam.css
    cp -v docs/code_documentation/vendor-logo.png public/
}


set_project_name(){
    cwd=$(pwd)
    cd public
    echo $(pwd)
    echo "Setting ${PROJECT_NAME}!"
    sed -i "s/\(PROJECT_NAME\)/${PROJECT_NAME}/g" index.html
    echo "Setting project URL!"
    sed -i "s'\(\#home\)'${PROJECT_URL}'g" index.html
    cd "$cwd"
}

copy_coverage(){
    if [ -d code_coverage_report ]; 
    then 
        cp -rv code_coverage_report public/
    else
        echo "ERROR. Directory: code_coverage_report does not exist! "
    fi
}

copy_readme(){
    if [ -f README.md ];
    then 
        cp -v README.md public/
        cwd=$(pwd)
        cd public
        echo $(pwd)
        markdown README.md --html4tags > about.html
        rm README.md
        cd "$cwd"
    else
        echo "ERROR. File: README.md does not exist! "
    fi
}

copy_documentation(){
    if [ -d docs/code_documentation ];
    then
        cwd=$(pwd)
        cd docs/
        echo $(pwd)
        cp -rv code_documentation ../public/
        cd "$cwd"
    else
         echo "ERROR. Directory: code_documentation does not exist! "
    fi
}

copy_packages(){
    if [ -d build/cpack_packages ];
    then
        tmp=(build/cpack_packages/*)
        for f in "${tmp[@]}"
        do
            f=${f##*/}
            FILE_LIST+=("$f")
        done
        cp -rv build/cpack_packages/* public/
    else 
        echo "WARNNING. No packages found!"
    fi
}

# Arg1 - link name 
# Arg2 - filename
link_file(){
    link_name=$1
    filename=$2
    lineNum="$(grep -n ${link_name} index.html | head -n 1 | cut -d: -f1)"
    lineNum=$((lineNum+1))
    pattern="<a id=\"${link_name}\" href=\"${filename}\">${filename}</a>"
    sed "${lineNum}i${pattern}" index.html > new_index.html
    rm index.html
    mv new_index.html index.html
}

write_files_to_index(){
    cwd=$(pwd)
    cd public
    echo $(pwd)
    for f in "${FILE_LIST[@]}"
    do
        if [[ "${f,,}" =~ "linux" ]];
        then
            link_file linux-release "${f}"
        elif [[ "${f,,}" =~ "window" ]];
        then
            link_file windows-release "${f}"
        else
            echo "$f is not a release file!"
        fi
    done
    cd "$cwd"
}

while [ -n "$1" ];
do
	case $1 in
		-n | --name)
			shift
			if [ -n "$1" ];
			then
				PROJECT_NAME=$1
			else
                echo "Project Name not provided. Aborting!"
				usage
				exit 1
			fi
			;;
        -l | --link)
            shift 
            if [ -n "$1" ];
			then
				PROJECT_URL=$1
			else
                echo "Project url not provided. Aborting!"
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

setup_env
setup_amalgam_index
set_project_name
copy_coverage
copy_readme
copy_documentation
copy_packages
write_files_to_index
