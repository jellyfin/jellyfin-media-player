###############################################################################
# CMake module to search for the mpv libraries.
#
# WARNING: This module is experimental work in progress.
#
# Based one FindVLC.cmake by:
# Copyright (c) 2011 Michael Jansen <info@michael-jansen.biz>
# Modified by Tobias Hieta <tobias@hieta.se>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################

#
### Global Configuration Section
#
SET(_BREAKPAD_REQUIRED_VARS BREAKPAD_INCLUDE_DIR BREAKPAD_LIBRARY)

#
### BREAKPAD uses pkgconfig.
#
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_BREAKPAD QUIET breakpad-client)
endif(PKG_CONFIG_FOUND)

#
### Look for the include files.
#
find_path(
    BREAKPAD_INCLUDE_DIR
    NAMES google_breakpad/common/breakpad_types.h
    PATH_SUFFIXES breakpad
    HINTS
        ${PC_BREAKPAD_INCLUDEDIR}
        ${PC_BREAKPAD_INCLUDE_DIRS} # Unused for BREAKPAD but anyway
    DOC "BREAKPAD include directory"
    )
mark_as_advanced(BREAKPAD_INCLUDE_DIR)
set(BREAKPAD_INCLUDE_DIRS ${BREAKPAD_INCLUDE_DIR})

#
### Look for the libraries (BREAKPAD and BREAKPADsore)
#
find_library(
    BREAKPAD_LIBRARY
    NAMES breakpad_client
    HINTS
        ${PC_BREAKPAD_LIBDIR}
        ${PC_BREAKPAD_LIBRARY_DIRS} # Unused for BREAKPAD but anyway
    PATH_SUFFIXES lib${LIB_SUFFIX}
    )
get_filename_component(_BREAKPAD_LIBRARY_DIR ${BREAKPAD_LIBRARY} PATH)
mark_as_advanced(BREAKPAD_LIBRARY)

set(BREAKPAD_LIBRARY_DIRS _BREAKPAD_CORE_LIBRARY_DIR _BREAKPAD_LIBRARY_DIR)
list(REMOVE_DUPLICATES BREAKPAD_LIBRARY_DIRS)
mark_as_advanced(BREAKPAD_LIBRARY_DIRS)

#
### Check if everything was found and if the version is sufficient.
#
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    BREAKPAD
    REQUIRED_VARS ${_BREAKPAD_REQUIRED_VARS}
    VERSION_VAR BREAKPAD_VERSION_STRING
    )

