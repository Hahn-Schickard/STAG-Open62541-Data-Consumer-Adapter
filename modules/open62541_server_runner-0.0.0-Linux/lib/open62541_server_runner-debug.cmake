#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OPCUA_Server" for configuration "Debug"
set_property(TARGET OPCUA_Server APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(OPCUA_Server PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libOPCUA_Server.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS OPCUA_Server )
list(APPEND _IMPORT_CHECK_FILES_FOR_OPCUA_Server "${_IMPORT_PREFIX}/lib/libOPCUA_Server.a" )

# Import target "Runner_MAIN" for configuration "Debug"
set_property(TARGET Runner_MAIN APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Runner_MAIN PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/Runner_MAIN"
  )

list(APPEND _IMPORT_CHECK_TARGETS Runner_MAIN )
list(APPEND _IMPORT_CHECK_FILES_FOR_Runner_MAIN "${_IMPORT_PREFIX}/bin/Runner_MAIN" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
