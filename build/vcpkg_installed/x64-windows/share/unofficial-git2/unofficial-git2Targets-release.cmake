#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::git2::git2" for configuration "Release"
set_property(TARGET unofficial::git2::git2 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::git2::git2 PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/git2.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/git2.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS unofficial::git2::git2 )
list(APPEND _IMPORT_CHECK_FILES_FOR_unofficial::git2::git2 "${_IMPORT_PREFIX}/lib/git2.lib" "${_IMPORT_PREFIX}/bin/git2.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
