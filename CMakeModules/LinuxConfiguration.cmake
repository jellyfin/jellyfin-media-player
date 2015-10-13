find_package(X11)
if(X11_FOUND AND X11_Xrandr_FOUND)
  include_directories(X11_X11_INCLUDE_PATH X11_Xrandr_INCLUDE_PATH)
  set(X11XRANDR_FOUND 1)
  add_definitions(-DUSE_X11XRANDR)
endif()

if (NOT BUILD_TARGET STREQUAL "RPI")
  set(USE_X11POWER ON)
  add_definitions(-DUSE_X11POWER)
endif()

set(INSTALL_BIN_DIR bin)
set(INSTALL_RESOURCE_DIR share)
