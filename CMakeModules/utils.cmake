#############################################################
function(get_resources_source_list target var)
  get_property(RESOURCE_LIST GLOBAL PROPERTY _${target}_RESOURCE_LIST)
  foreach(RF ${RESOURCE_LIST})
    string(REPLACE "|" ";" PARTS "${RF}")
    list(GET PARTS 0 SOURCE_FILE)
    list(APPEND _SF ${SOURCE_FILE})
  endforeach()
  set(${var} ${_SF} PARENT_SCOPE)
endfunction()

#############################################################
function(copy_resources target)
  if(XCODE)
    return()
  endif()
  
  get_property(RESOURCE_LIST GLOBAL PROPERTY _${target}_RESOURCE_LIST)

  # we read the LOCATION from the target instead of using a generator
  # here since add_custom_command doesn't support generator expresessions
  # in the output field, and this is still cleaner than hardcoding the path
  # of the output binary.
  #  
  get_property(TARGET_LOC TARGET ${target} PROPERTY LOCATION)
  get_filename_component(TARGET_DIR ${TARGET_LOC} DIRECTORY)
  if(APPLE)
    set(TARGET_LOC ${TARGET_DIR}/..)
  else()
    set(TARGET_LOC ${TARGET_DIR})
  endif()
  
  if(RESOURCE_LIST)
    foreach(RF ${RESOURCE_LIST})
      string(REPLACE "|" ";" PARTS "${RF}")
      list(GET PARTS 0 SOURCE_FILE)
      list(GET PARTS 1 _TARGET_FILE)
      set(TARGET_FILE ${TARGET_LOC}/${_TARGET_FILE})
      add_custom_command(OUTPUT ${TARGET_FILE}
                         COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE_FILE}" "${TARGET_FILE}"
                         DEPENDS ${SOURCE_FILE}
                         COMMENT "CopyResource (${target}): ${TARGET_FILE}")
      list(APPEND RESOURCES ${TARGET_FILE})
    endforeach()
    add_custom_target(${target}_CopyResources DEPENDS ${RESOURCES})
    add_dependencies(${target} ${target}_CopyResources)
  endif(RESOURCE_LIST)
endfunction()

#############################################################
function(add_resources)
  set(args1 TARGET)
  set(args2 SOURCES DEST EXCLUDE)
  cmake_parse_arguments(BD "" "${args1}" "${args2}" ${ARGN})
  
  foreach(_BDFILE ${BD_SOURCES})
    if(IS_DIRECTORY ${_BDFILE})
      file(GLOB _DIRCONTENTS ${_BDFILE}/*)
      foreach(_BDDFILE ${_DIRCONTENTS})
        get_filename_component(_BDFILE_NAME ${_BDDFILE} NAME)

        set(PROCESS_FILE 1)
        foreach(EX_FILE ${BD_EXCLUDE})
          string(REGEX MATCH ${EX_FILE} DID_MATCH ${_BDDFILE})
          if(NOT "${DID_MATCH}" STREQUAL "")
            set(PROCESS_FILE 0)
          endif(NOT "${DID_MATCH}" STREQUAL "")
        endforeach(EX_FILE ${BD_EXCLUDE})

        if(PROCESS_FILE STREQUAL "1")
          if(IS_DIRECTORY ${_BDDFILE})
            set(DEST ${BD_DEST}/${_BDFILE_NAME})
          else()
            set(DEST ${BD_DEST})
          endif()
          
          add_resources(SOURCES ${_BDDFILE} DEST ${DEST} EXCLUDE ${BD_EXCLUDE} TARGET ${BD_TARGET})
        endif()
      endforeach()
    else()
      get_filename_component(_BDFILE_NAME ${_BDFILE} NAME)
      set_property(GLOBAL APPEND PROPERTY _${BD_TARGET}_RESOURCE_LIST "${_BDFILE}|${BD_DEST}/${_BDFILE_NAME}")
      if(XCODE)
        set_source_files_properties(${_BDFILE} PROPERTIES MACOSX_PACKAGE_LOCATION ${BD_DEST})
      endif()
    endif()    
  endforeach()
endfunction()

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
  set_target_properties(${target} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
endfunction()

#############################################################
function(safe_download URL)
  set(ARGS FILENAME SHA1)
  cmake_parse_arguments(SD "SHOW_PROGRESS" "${ARGS}" "" ${ARGN})
  if(NOT DEFINED SD_FILENAME)
    get_filename_component(SD_FILENAME ${CU_URL} NAME)
  endif(NOT DEFINED SD_FILENAME)

  if(EXISTS "${SD_FILENAME}")
    file(SHA1 "${SD_FILENAME}" CURRENT_SHA1)
  endif()

  if(NOT DEFINED SD_SHA1)
    set(SD_SHA1 ${CURRENT_SHA1})
  endif()

  if(SD_SHOW_PROGRESS)
    set(_SHOW_PROGRESS "SHOW_PROGRESS")
  endif()

  if(NOT CURRENT_SHA1 STREQUAL SD_SHA1)
    message(STATUS "Downloading ${URL} to ${SD_FILENAME}...")
    file(
      DOWNLOAD ${URL} ${SD_FILENAME}
      STATUS SD_STATUS
      ${_SHOW_PROGRESS}
    )
    list(GET SD_STATUS 0 SD_SUCCESS)
    if(NOT SD_SUCCESS EQUAL 0)
      list(GET SD_STATUS 1 SD_ERROR)
      file(REMOVE ${SD_FILENAME})
      if("${SD_ERROR}" STREQUAL "\"Unsupported protocol\"")
        message(FATAL_ERROR "Download failed and your cmake probably don't support SSL! Beware that cmake downloaded from cmake.org doesn't support SSL on all platforms, make sure to build it yourself.")
      endif()
      message(FATAL_ERROR "Failed to download: ${URL}: ${SD_ERROR}")
    endif()

    file(SHA1 "${SD_FILENAME}" NEW_SHA1)
    if(DEFINED SD_SHA1)
      if(NOT SD_SHA1 STREQUAL NEW_SHA1)
        file(REMOVE ${SD_FILENAME})
        message(FATAL_ERROR "Failed to verify SHA1 on ${SD_FILENAME}, expected '${SD_SHA1}' got '${NEW_SHA1}'")
      endif()
    endif()
  endif()
endfunction()
