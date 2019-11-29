#!/bin/bash

EXECUTABLE_FILE=""
ARGUMENTS=""

usage(){
    echo "-e | --executable   - (Mandatory) Specify the binary executable file to run valgrind checks on."
    echo "-a | --arguments    - (Optional) Specify the executable files argument (for example: Arg1 Arg2)"
    echo "-h | --help         - Print this message"
}

check_executable() {
    if [ ! -f "$EXECUTABLE_FILE" ];
    then
        echo "Executable file: $EXECUTABLE_FILE was not found. Aborting!"
        exit 1
    fi
}

run_valgrind(){
    if [ -d valgrind_results ]
    then 
        rm -rf valgrind_results
    fi
    mkdir valgrind_results
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose $EXECUTABLE_FILE $ARGUMENTS 2>&1 | tee valgrind_results/valgrind-results.log 
}

check_for_errors() {
    HEAD_REGEX="\(ERROR SUMMARY:.\)"
    TAIL_REGEX="\(\s.*\)"
    found_errors=$(sed -n "s/^\(.*\)${HEAD_REGEX}//p" valgrind_results/valgrind-results.log)
    found_errors=$(sed "s/$TAIL_REGEX//" <<< $found_errors)
    found_errors=$(sed '$d' <<< $found_errors)
    if [[ "$found_errors" -ne 0 ]];
    then 
        echo "Error checker found: $found_errors"
        echo "Some errors exits! Please fix them before doing a release!" 
        exit 1
    fi
}

while [ -n "$1" ];
do
	case $1 in
		-e | --executable)
			shift
			if [ -n "$1" ];
			then
				EXECUTABLE_FILE=$1
			else
                echo "Executable not provided. Aborting!"
				usage
				exit 1
			fi
			;;
        -a | --arguments)
            shift
            ARGUMENTS=$1
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

check_executable
run_valgrind
check_for_errors
