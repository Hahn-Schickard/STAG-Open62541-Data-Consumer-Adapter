set(THIS Model_Translator)

add_subdirectory(Utilities)

if(HISTORIZATION)
    add_subdirectory(Historizer)
endif(HISTORIZATION)

add_subdirectory(Server)
add_subdirectory(Adapter)
add_subdirectory(Example)
