# - find where dlopen and friends are located.
# DL_FOUND - system has dynamic linking interface available
# DL_INCLUDE_DIR - where dlfcn.h is located.
# DL_LIBRARIES - libraries needed to use dlopen

include(CheckFunctionExists)

find_path(DL_INCLUDE_DIR NAMES dlfcn.h)
find_library(DL_LIBRARIES NAMES dl)
if(DL_LIBRARIES)
  set(DL_FOUND)
else(DL_LIBRARIES)
  check_function_exists(dlopen DL_FOUND)
  # If dlopen can be found without linking in dl then dlopen is part
  # of libc, so don't need to link extra libs.
  set(DL_LIBRARIES "")
endif(DL_LIBRARIES)
