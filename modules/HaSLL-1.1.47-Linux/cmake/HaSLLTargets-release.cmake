#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "HaSLL::spdlog" for configuration "Release"
set_property(TARGET HaSLL::spdlog APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(HaSLL::spdlog PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libspdlog.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS HaSLL::spdlog )
list(APPEND _IMPORT_CHECK_FILES_FOR_HaSLL::spdlog "${_IMPORT_PREFIX}/lib/libspdlog.a" )

# Import target "HaSLL::Logger" for configuration "Release"
set_property(TARGET HaSLL::Logger APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(HaSLL::Logger PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libLogger.so"
  IMPORTED_SONAME_RELEASE "libLogger.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS HaSLL::Logger )
list(APPEND _IMPORT_CHECK_FILES_FOR_HaSLL::Logger "${_IMPORT_PREFIX}/lib/libLogger.so" )

# Import target "HaSLL::EXAMPLE" for configuration "Release"
set_property(TARGET HaSLL::EXAMPLE APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(HaSLL::EXAMPLE PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/EXAMPLE"
  )

list(APPEND _IMPORT_CHECK_TARGETS HaSLL::EXAMPLE )
list(APPEND _IMPORT_CHECK_FILES_FOR_HaSLL::EXAMPLE "${_IMPORT_PREFIX}/bin/EXAMPLE" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
