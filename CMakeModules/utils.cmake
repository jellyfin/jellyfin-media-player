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
