#!/bin/bash 

if [ -f ./code_coverage.info ]
then
    rm ./code_coverage.info 
fi 

if [ -d code_coverage_report ]
then 
    rm -rf code_coverage_report
fi

lcov --directory build/ --capture --output-file ./code_coverage.info -rc lcov_branch_coverage=1 
lcov --remove code_coverage.info "/usr*" -o code_coverage.info
lcov --remove code_coverage.info "*/googletest-src/*" -o code_coverage.info
lcov --remove code_coverage.info "*/spdlog/*" -o code_coverage.info
lcov --remove code_coverage.info "*/libs/*" -o code_coverage.info
lcov --list code_coverage.info
genhtml code_coverage.info --branch-coverage --output-directory ./code_coverage_report/
