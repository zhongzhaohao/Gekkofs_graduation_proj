################################################################################
# Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# SPDX-License-Identifier: MIT                                                 #
################################################################################

find_path(DATE_INCLUDE_DIR
    NAMES date/date.h
)

find_path(TZ_INCLUDE_DIR
    NAMES date/tz.h
)

find_library(TZ_LIBRARY
    NAMES tz
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( Date 
    DEFAULT_MSG 
    DATE_INCLUDE_DIR
    TZ_INCLUDE_DIR
    TZ_LIBRARY
)

if(Date_FOUND)
    set(DATE_INCLUDE_DIRS ${DATE_INCLUDE_DIR})
    set(TZ_INCLUDE_DIRS ${TZ_INCLUDE_DIR})
    set(TZ_LIBRARIES ${TZ_LIBRARY})

    if(NOT TARGET Date::TZ)
        add_library(Date::TZ UNKNOWN IMPORTED)
        set_target_properties(Date::TZ PROPERTIES
            IMPORTED_LOCATION "${TZ_LIBRARY}"
            INTERFACE_COMPILE_DEFINITIONS "USE_OS_TZDB=1"
            INTERFACE_INCLUDE_DIRECTORIES "${TZ_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(
    DATE_INCLUDE_DIR
    TZ_INCLUDE_DIR
    TZ_LIBRARY
)
