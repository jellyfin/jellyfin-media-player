if(CEC_INCLUDE_DIR)
  # Already in cache, be silent
  set(CEC_FIND_QUIETLY TRUE)
endif(CEC_INCLUDE_DIR)

if (PKG_CONFIG_FOUND)
  pkg_check_modules(_CEC QUIET libcec>=2.0)
endif (PKG_CONFIG_FOUND)

Find_Path(CEC_INCLUDE_DIR
  NAMES cec/cec.h libcec/cec.h
  PATHS /usr/include usr/local/include 
  PATH_SUFFIXES cec cec/cec libcec cec/include
  HINTS ${_CEC_INCLUDEDIR}
)

Find_Library(CEC_LIBRARY
  NAMES cec
  PATHS /usr/lib usr/local/lib
  PATH_SUFFIXES cec libcec cec/lib libcec/lib
  HINTS ${_CEC_LIBDIR} 
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CEC DEFAULT_MSG CEC_LIBRARY CEC_INCLUDE_DIR)


