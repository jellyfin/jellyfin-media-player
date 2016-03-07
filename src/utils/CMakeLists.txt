
add_sources(
  CachedRegexMatcher.cpp CachedRegexMatcher.h
  PlatformUtils.cpp PlatformUtils.h
  HelperLauncher.h HelperLauncher.cpp
  Utils.cpp Utils.h
  Log.cpp Log.h
)

if(APPLE)
  add_sources(HelperLaunchd.cpp HelperLaunchd.cpp)
  add_subdirectory(osx)
endif(APPLE)
