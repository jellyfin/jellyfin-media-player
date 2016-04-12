set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS_THIRD_PARTY}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS_THIRD_PARTY}")

add_subdirectory(qslog)
add_subdirectory(qhttp)

if(APPLE)
  add_subdirectory(plistparser)
  add_subdirectory(SPMediaKeyTap)
  add_subdirectory(HIDRemote)
endif(APPLE)
