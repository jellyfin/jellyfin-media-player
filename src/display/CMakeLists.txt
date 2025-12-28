if(APPLE)
  add_subdirectory(osx)
elseif(X11XRANDR_FOUND)
  add_subdirectory(x11)
elseif(WIN32)
  add_subdirectory(win)
endif()

add_subdirectory(dummy)

find_all_sources(. SRC)
add_sources(${SRC})