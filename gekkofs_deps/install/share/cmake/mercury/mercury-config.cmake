#-----------------------------------------------------------------------------
# mercury-config.cmake - Mercury CMake configuration file for external projects.
#-----------------------------------------------------------------------------
set(__mercury_install_tree TRUE)
if(__mercury_install_tree)
  get_filename_component(location "${CMAKE_CURRENT_LIST_FILE}" PATH)
  set(MERCURY_CONFIG_TARGETS_FILE "${location}/mercury-targets.cmake")
else()
  # This is the build location.
  set(MERCURY_CONFIG_TARGETS_FILE "/home/changqin/gekkofs/gekkofs_deps/git/mercury/build/src/mercury-targets.cmake")
endif()

#-----------------------------------------------------------------------------
# User Options
#-----------------------------------------------------------------------------
set(MERCURY_BUILD_SHARED_LIBS    ON)
set(MERCURY_USE_BOOST_PP         ON)
set(MERCURY_USE_CHECKSUMS        OFF)
set(MERCURY_USE_SYSTEM_MCHECKSUM )

#-----------------------------------------------------------------------------
# Version information for Mercury
#-----------------------------------------------------------------------------
set(MERCURY_VERSION_MAJOR   2)
set(MERCURY_VERSION_MINOR   1)
set(MERCURY_VERSION_PATCH   0)
set(MERCURY_VERSION_FULL    2.1.0)
set(MERCURY_VERSION         2.1)

#-----------------------------------------------------------------------------
# Don't include targets if this file is being picked up by another
# project which has already built MERCURY as a subproject
#-----------------------------------------------------------------------------
if(NOT MERCURY_INSTALL_SKIP_TARGETS)
  if(NOT TARGET "mchecksum" AND MERCURY_USE_CHECKSUMS AND MERCURY_USE_SYSTEM_MCHECKSUM)
    include(/mchecksum-config.cmake)
  endif()
  if(NOT TARGET "mercury")
    include(${MERCURY_CONFIG_TARGETS_FILE})
  endif()
endif()

# cleanup
unset(__mercury_install_tree)
