add_sources(
  ComponentManager.cpp ComponentManager.h
  ProfileManager.cpp ProfileManager.h
  Globals.cpp Globals.h
  Version.h
)

if(UNIX)
  add_sources(SignalManager.cpp SignalManager.h)
endif()
