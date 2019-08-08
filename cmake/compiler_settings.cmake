if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(STATUS "Using Clang compiler")
    # setup your Clang toolchain here
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "Using GNU compiler")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage")
    
    add_definitions(
        -W
        -Wunused-variable 
        -Wunused-parameter 
        -Wunused-function 
        -Wunused 
        -Wno-system-headers
        -Wno-deprecated 
        -Woverloaded-virtual 
        -Wwrite-strings
        -Wall
        -fprofile-arcs 
        -ftest-coverage
        -fdiagnostics-color=auto
        -lgcov
        --coverage
    )

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # setup your MSVC toolchain here
endif()