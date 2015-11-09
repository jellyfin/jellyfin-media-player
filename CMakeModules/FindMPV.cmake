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
SET(_MPV_REQUIRED_VARS MPV_INCLUDE_DIR MPV_LIBRARY)

#
### MPV uses pkgconfig.
#
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_MPV QUIET mpv)
endif(PKG_CONFIG_FOUND)


#
### Look for the include files.
#
find_path(
  MPV_INCLUDE_DIR
  NAMES mpv/client.h
  HINTS
      ${PC_MPV_INCLUDEDIR}
      ${PC_MPV_INCLUDE_DIRS} # Unused for MPV but anyway
  DOC "MPV include directory"
)

#
### Look for the libraries
#
set(_MPV_LIBRARY_NAMES mpv)
if(PC_MPV_LIBRARIES)
  set(_MPV_LIBRARY_NAMES ${PC_MPV_LIBRARIES})
endif(PC_MPV_LIBRARIES)

foreach(l ${_MPV_LIBRARY_NAMES})
  find_library(
    MPV_LIBRARY_${l}
    NAMES ${l}
    HINTS
      ${PC_MPV_LIBDIR}
      ${PC_MPV_LIBRARY_DIRS} # Unused for MPV but anyway
    PATH_SUFFIXES lib${LIB_SUFFIX}
  )
  list(APPEND MPV_LIBRARY ${MPV_LIBRARY_${l}})
endforeach()

get_filename_component(_MPV_LIBRARY_DIR ${MPV_LIBRARY_mpv} PATH)
mark_as_advanced(MPV_LIBRARY)

set(MPV_LIBRARY_DIRS _MPV_LIBRARY_DIR)
list(REMOVE_DUPLICATES MPV_LIBRARY_DIRS)

mark_as_advanced(MPV_INCLUDE_DIR)
mark_as_advanced(MPV_LIBRARY_DIRS)
set(MPV_INCLUDE_DIRS ${MPV_INCLUDE_DIR})

#
### Check if everything was found and if the version is sufficient.
#
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  MPV
  REQUIRED_VARS ${_MPV_REQUIRED_VARS}
  VERSION_VAR MPV_VERSION_STRING
)

