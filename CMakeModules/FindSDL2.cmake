###############################################################################
# CMake module to search for the SDL libraries.
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
SET(_SDL2_REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARY)

#
### SDL uses pkgconfig.
#
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SDL QUIET sdl2)
endif(PKG_CONFIG_FOUND)

#
### Look for the include files.
#
find_path(
    SDL2_INCLUDE_DIR
    NAMES SDL.h
    PATH_SUFFIXES SDL2
    HINTS
        ${PC_SDL2_INCLUDEDIR}
        ${PC_SDL2_INCLUDE_DIRS} # Unused for SDL but anyway
    DOC "SDL2 include directory"
    )
mark_as_advanced(SDL2_INCLUDE_DIR)
set(SDL2_INCLUDE_DIRS ${SDL_INCLUDE_DIR})

#
### Look for the libraries (SDL and SDLsore)
#
find_library(
    SDL2_LIBRARY
    NAMES SDL2
    HINTS
        ${PC_SDL2_LIBDIR}
        ${PC_SDL2_LIBRARY_DIRS} # Unused for SDL but anyway
    PATH_SUFFIXES lib${LIB_SUFFIX}
    )
get_filename_component(_SDL2_LIBRARY_DIR "${SDL2_LIBRARY}" PATH)
mark_as_advanced(SDL2_LIBRARY)

set(SDL2_LIBRARY_DIRS _SDL2_LIBRARY_DIR)
list(REMOVE_DUPLICATES SDL2_LIBRARY_DIRS)
mark_as_advanced(SDL2_LIBRARY_DIRS)

#
### Check if everything was found and if the version is sufficient.
#
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    SDL2
    REQUIRED_VARS ${_SDL2_REQUIRED_VARS}
    VERSION_VAR SDL2_VERSION_STRING
    )

