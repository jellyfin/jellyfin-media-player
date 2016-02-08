add_subdirectory(qslog)
add_subdirectory(qhttp)

if(APPLE)
  add_subdirectory(plistparser)
  add_subdirectory(SPMediaKeyTap)
endif(APPLE)
