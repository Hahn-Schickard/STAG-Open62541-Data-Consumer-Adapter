find_program(CLANG_TIDY "clang-tidy")

if(CLANG_TIDY)
set(CMAKE_CXX_CLANG_TIDY
    ${suppress_exit_status};
    clang-tidy;
    -header-filter=${includes_dir};
    -export-fixes=${clang_output}
    --add-compiler-defaults)
endif()