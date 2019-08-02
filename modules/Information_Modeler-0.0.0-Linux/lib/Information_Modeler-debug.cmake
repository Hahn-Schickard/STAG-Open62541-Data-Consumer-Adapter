#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Information_Model" for configuration "Debug"
set_property(TARGET Information_Model APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Information_Model PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libInformation_Model.so"
  IMPORTED_SONAME_DEBUG "libInformation_Model.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Information_Model )
list(APPEND _IMPORT_CHECK_FILES_FOR_Information_Model "${_IMPORT_PREFIX}/lib/libInformation_Model.so" )

# Import target "Model_Factory" for configuration "Debug"
set_property(TARGET Model_Factory APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Model_Factory PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libModel_Factory.so"
  IMPORTED_SONAME_DEBUG "libModel_Factory.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Model_Factory )
list(APPEND _IMPORT_CHECK_FILES_FOR_Model_Factory "${_IMPORT_PREFIX}/lib/libModel_Factory.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
