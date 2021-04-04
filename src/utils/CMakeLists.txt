
add_sources(
  CachedRegexMatcher.cpp CachedRegexMatcher.h
  PlatformUtils.cpp PlatformUtils.h
  Utils.cpp Utils.h
  Log.cpp Log.h
)

if(APPLE)
  add_subdirectory(osx)
endif(APPLE)

if(WIN32)
  add_subdirectory(win)
endif()
