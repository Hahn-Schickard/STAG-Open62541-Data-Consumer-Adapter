#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "InfoModel_Lib" for configuration "Debug"
set_property(TARGET InfoModel_Lib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(InfoModel_Lib PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libInfoModel_Lib.so"
  IMPORTED_SONAME_DEBUG "libInfoModel_Lib.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS InfoModel_Lib )
list(APPEND _IMPORT_CHECK_FILES_FOR_InfoModel_Lib "${_IMPORT_PREFIX}/lib/libInfoModel_Lib.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
