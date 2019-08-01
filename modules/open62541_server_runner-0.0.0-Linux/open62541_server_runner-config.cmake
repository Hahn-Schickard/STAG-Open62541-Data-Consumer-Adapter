get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/lib/open62541_server_runner.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/modules/open62541-build/open62541Config.cmake)