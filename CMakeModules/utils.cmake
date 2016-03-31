#############################################################
function(set_bundle_dir)
  set(args SOURCES DEST EXCLUDE)
  include(CMakeParseArguments)
  cmake_parse_arguments(BD "" "" "${args}" ${ARGN})

  foreach(_BDIR ${BD_SOURCES}) 
    file(GLOB _DIRCONTENTS ${_BDIR}/*)
    foreach(_BDFILE ${_DIRCONTENTS})
      get_filename_component(_BDFILE_NAME ${_BDFILE} NAME)

      set(PROCESS_FILE 1)
      foreach(EX_FILE ${BD_EXCLUDE})
        string(REGEX MATCH ${EX_FILE} DID_MATCH ${_BDFILE})
        if(NOT "${DID_MATCH}" STREQUAL "")
          set(PROCESS_FILE 0)
        endif(NOT "${DID_MATCH}" STREQUAL "")
      endforeach(EX_FILE ${BD_EXCLUDE})
      
      if(PROCESS_FILE STREQUAL "1")
        if(IS_DIRECTORY ${_BDFILE})
          set_bundle_dir(SOURCES ${_BDFILE} DEST ${BD_DEST}/${_BDFILE_NAME} EXCLUDE ${BD_EXCLUDE})
        else(IS_DIRECTORY ${_BDFILE})
          #message("set_bundle_dir : setting package_location ${_BDFILE} = ${BD_DEST}")
          set_source_files_properties(${_BDFILE} PROPERTIES MACOSX_PACKAGE_LOCATION ${BD_DEST})
          get_property(BUNDLED_FILES GLOBAL PROPERTY CONFIG_BUNDLED_FILES)
          set_property(GLOBAL PROPERTY CONFIG_BUNDLED_FILES ${BUNDLED_FILES} ${_BDFILE})

          string(REPLACE "/" "\\\\" GNAME ${BD_DEST})
          source_group(${GNAME} FILES ${_BDFILE})
        endif(IS_DIRECTORY ${_BDFILE})
      endif()
    endforeach(_BDFILE ${_DIRCONTENTS})
  endforeach(_BDIR ${BD_SOURCES})
endfunction(set_bundle_dir)

#############################################################
macro(find_all_sources DIRECTORY VARIABLE)
  aux_source_directory(${DIRECTORY} ${VARIABLE})
  file(GLOB headers ${DIRECTORY}/*h)
  list(APPEND ${VARIABLE} ${headers})
endmacro()

#############################################################
# function to collect all the sources from sub-directories
# into a single list
function(add_sources)
  get_property(is_defined GLOBAL PROPERTY SRCS_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY SRCS_LIST
      BRIEF_DOCS "List of source files"
      FULL_DOCS "List of source files to be compiled in one library")
  endif()
  # make absolute paths
  set(SRCS)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND SRCS "${s}")
  endforeach()

  string(REPLACE ${CMAKE_SOURCE_DIR}/src/ "" SUBDIR ${CMAKE_CURRENT_SOURCE_DIR})
  string(TOLOWER ${SUBDIR} SUBDIR)
  string(REPLACE "/" "\\\\" LIBNAME ${SUBDIR})
  source_group(${LIBNAME} FILES ${SRCS})

  # add it to the global list.
  set_property(GLOBAL APPEND PROPERTY SRCS_LIST ${SRCS})
endfunction(add_sources)

## ---------------------------------------------------------------------
##
## Copyright (C) 2012 - 2014 by the deal.II authors
##
## This file is part of the deal.II library.
##
## The deal.II library is free software; you can use it, redistribute
## it, and/or modify it under the terms of the GNU Lesser General
## Public License as published by the Free Software Foundation; either
## version 2.1 of the License, or (at your option) any later version.
## The full text of the license can be found in the file LICENSE at
## the top level of the deal.II distribution.
##
## ---------------------------------------------------------------------

#
# Tests whether the cxx compiler understands a flag.
# If so, add it to 'variable'.
#
# Usage:
#     ENABLE_IF_SUPPORTED(variable flag)
#

include(CheckCXXCompilerFlag)

MACRO(ENABLE_IF_SUPPORTED _variable _flag)
  #
  # Clang is too conservative when reporting unsupported compiler flags.
  # Therefore, we promote all warnings for an unsupported compiler flag to
  # actual errors with the -Werror switch:
  #
  IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    SET(_werror_string "-Werror ")
  ELSE()
    SET(_werror_string "")
  ENDIF()

  STRING(STRIP "${_flag}" _flag_stripped)
  SET(_flag_stripped_orig "${_flag_stripped}")

  #
  # Gcc does not emit a warning if testing -Wno-... flags which leads to
  # false positive detection. Unfortunately it later warns that an unknown
  # warning option is used if another warning is emitted in the same
  # compilation unit.
  # Therefore we invert the test for -Wno-... flags:
  #
  IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    STRING(REPLACE "-Wno-" "-W" _flag_stripped "${_flag_stripped}")
  ENDIF()

  IF(NOT "${_flag_stripped}" STREQUAL "")
    STRING(REGEX REPLACE "^-" "" _flag_name "${_flag_stripped}")
    STRING(REPLACE "," "" _flag_name "${_flag_name}")
    STRING(REPLACE "-" "_" _flag_name "${_flag_name}")
    STRING(REPLACE "+" "_" _flag_name "${_flag_name}")
    CHECK_CXX_COMPILER_FLAG(
      "${_werror_string}${_flag_stripped}"
      HAVE_FLAG_${_flag_name}
    )
    IF(HAVE_FLAG_${_flag_name})
      SET(${_variable} "${${_variable}} ${_flag_stripped_orig}")
      STRING(STRIP "${${_variable}}" ${_variable})
    ENDIF()
  ENDIF()
ENDMACRO()

#
# Tests whether it is possible to compile and link a dummy program with a
# given flag.
# If so, add it to variable.
#
# Usage:
#     ENABLE_IF_LINKS(variable flag)
#

MACRO(ENABLE_IF_LINKS _variable _flag)
  STRING(STRIP "${_flag}" _flag_stripped)
  IF(NOT "${_flag_stripped}" STREQUAL "")
    STRING(REGEX REPLACE "^-" "" _flag_name "${_flag_stripped}")
    STRING(REPLACE "," "" _flag_name "${_flag_name}")
    STRING(REPLACE "-" "_" _flag_name "${_flag_name}")
    STRING(REPLACE "+" "_" _flag_name "${_flag_name}")
    SET(_backup ${CMAKE_REQUIRED_LIBRARIES})
    LIST(APPEND CMAKE_REQUIRED_LIBRARIES "${_flag_stripped}")
    CHECK_CXX_COMPILER_FLAG(
      ""
      HAVE_FLAG_${_flag_name}
    )
    SET(CMAKE_REQUIRED_LIBRARIES ${_backup})

    IF(HAVE_FLAG_${_flag_name})
      SET(${_variable} "${${_variable}} ${_flag_stripped}")
      STRING(STRIP "${${_variable}}" ${_variable})
    ENDIF()
  ENDIF()
ENDMACRO()

#############################################################
function(std_target_properties target)
  set_target_properties(${target} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
endfunction()
