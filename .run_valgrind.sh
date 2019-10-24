#!/bin/bash

EXECUTABLE_FILE=""
ARGUMENTS=""

usage(){
    echo "-e | --executable   - (Mandatory) Specify the binary executable file to run valgrind checks on."
    echo "-a | --arguments    - (Optional) Specify the executable files argument (for example: Arg1 Arg2)"
    echo "-h | --help         - Print this message"
}

run_valgrind(){
    if [ -f valgrind-results.log ]
    then 
        rm valgrind-results.log
    fi
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose $EXECUTABLE_FILE $ARGUMENTS 2>&1 | tee valgrind-results.log 
}

check_for_errors() {
    HEAD_REGEX="\(ERROR SUMMARY:.\)"
    TAIL_REGEX="\(\s.*\)"
    found_errors=$(sed -n "s/^\(.*\)${HEAD_REGEX}//p" ./valgrind-results.log)
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

run_valgrind
check_for_errors
