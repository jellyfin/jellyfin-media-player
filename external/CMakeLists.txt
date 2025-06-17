set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS_THIRD_PARTY}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS_THIRD_PARTY}")

add_subdirectory(discord)

if(APPLE)
  add_subdirectory(plistparser)
  add_subdirectory(HIDRemote)
  add_subdirectory(letsmove)
endif(APPLE)

