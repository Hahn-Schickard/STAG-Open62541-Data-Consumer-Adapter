#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OPCUA_Server" for configuration "Debug"
set_property(TARGET OPCUA_Server APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(OPCUA_Server PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libOPCUA_Server.so"
  IMPORTED_SONAME_DEBUG "libOPCUA_Server.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS OPCUA_Server )
list(APPEND _IMPORT_CHECK_FILES_FOR_OPCUA_Server "${_IMPORT_PREFIX}/lib/libOPCUA_Server.so" )

# Import target "Server_Runner" for configuration "Debug"
set_property(TARGET Server_Runner APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Server_Runner PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "OPCUA_Server"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libServer_Runner.so"
  IMPORTED_SONAME_DEBUG "libServer_Runner.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Server_Runner )
list(APPEND _IMPORT_CHECK_FILES_FOR_Server_Runner "${_IMPORT_PREFIX}/lib/libServer_Runner.so" )

# Import target "Runner_MAIN" for configuration "Debug"
set_property(TARGET Runner_MAIN APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Runner_MAIN PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/Runner_MAIN"
  )

list(APPEND _IMPORT_CHECK_TARGETS Runner_MAIN )
list(APPEND _IMPORT_CHECK_FILES_FOR_Runner_MAIN "${_IMPORT_PREFIX}/bin/Runner_MAIN" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
