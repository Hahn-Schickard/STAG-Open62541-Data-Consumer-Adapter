if(STATIC_CODE_ANALYSIS)
  message(STATUS "Running Static analysis during build!")
  find_program(CLANG_TIDY_EXE
    NAMES
      "clang-tidy-19"
      "clang-tidy-18"
      "clang-tidy-17"
      "clang-tidy-16"
      "clang-tidy-15"
      "clang-tidy"
  )
  if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy!")
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}/compile_commands.json;-export-fixes=${CLANG_OUTPUT}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}/compile_commands.json;-export-fixes=${CLANG_OUTPUT}")
  endif()
else()
  message(STATUS "Code Linting is disabled via CMake Option")
endif()
