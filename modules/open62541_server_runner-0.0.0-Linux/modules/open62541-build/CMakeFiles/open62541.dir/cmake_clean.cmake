file(REMOVE_RECURSE
  "bin/libopen62541.pdb"
  "bin/libopen62541.so"
  "bin/libopen62541.so.1.0.0"
  "bin/libopen62541.so.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/open62541.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
