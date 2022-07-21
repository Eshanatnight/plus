#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::http_parser::http_parser" for configuration "Debug"
set_property(TARGET unofficial::http_parser::http_parser APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::http_parser::http_parser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/http_parser.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS unofficial::http_parser::http_parser )
list(APPEND _IMPORT_CHECK_FILES_FOR_unofficial::http_parser::http_parser "${_IMPORT_PREFIX}/debug/lib/http_parser.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
