add_subdirectory(qhttpserver)
add_subdirectory(qslog)

if(APPLE)
  add_subdirectory(plistparser)
  add_subdirectory(SPMediaKeyTap)
endif(APPLE)