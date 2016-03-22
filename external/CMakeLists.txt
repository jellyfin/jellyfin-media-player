add_subdirectory(qslog)
add_subdirectory(qhttp)

if(APPLE)
  add_subdirectory(plistparser)
  add_subdirectory(SPMediaKeyTap)
  add_subdirectory(HIDRemote)
endif(APPLE)
