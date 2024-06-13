#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mercury_util" for configuration "Release"
set_property(TARGET mercury_util APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mercury_util PROPERTIES
  IMPORTED_LOCATION_RELEASE "/home/changqin/gekkofs/gekkofs_deps/install/lib/libmercury_util.so.2.1.0"
  IMPORTED_SONAME_RELEASE "libmercury_util.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS mercury_util )
list(APPEND _IMPORT_CHECK_FILES_FOR_mercury_util "/home/changqin/gekkofs/gekkofs_deps/install/lib/libmercury_util.so.2.1.0" )

# Import target "na" for configuration "Release"
set_property(TARGET na APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(na PROPERTIES
  IMPORTED_LOCATION_RELEASE "/home/changqin/gekkofs/gekkofs_deps/install/lib/libna.so.2.1.0"
  IMPORTED_SONAME_RELEASE "libna.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS na )
list(APPEND _IMPORT_CHECK_FILES_FOR_na "/home/changqin/gekkofs/gekkofs_deps/install/lib/libna.so.2.1.0" )

# Import target "mercury" for configuration "Release"
set_property(TARGET mercury APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mercury PROPERTIES
  IMPORTED_LOCATION_RELEASE "/home/changqin/gekkofs/gekkofs_deps/install/lib/libmercury.so.2.1.0"
  IMPORTED_SONAME_RELEASE "libmercury.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS mercury )
list(APPEND _IMPORT_CHECK_FILES_FOR_mercury "/home/changqin/gekkofs/gekkofs_deps/install/lib/libmercury.so.2.1.0" )

# Import target "mercury_hl" for configuration "Release"
set_property(TARGET mercury_hl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mercury_hl PROPERTIES
  IMPORTED_LOCATION_RELEASE "/home/changqin/gekkofs/gekkofs_deps/install/lib/libmercury_hl.so.2.1.0"
  IMPORTED_SONAME_RELEASE "libmercury_hl.so.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS mercury_hl )
list(APPEND _IMPORT_CHECK_FILES_FOR_mercury_hl "/home/changqin/gekkofs/gekkofs_deps/install/lib/libmercury_hl.so.2.1.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
