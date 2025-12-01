add_sources(PowerComponent.h PowerComponent.cpp)

if(APPLE)
  add_sources(PowerComponentMac.cpp PowerComponentMac.h)
endif(APPLE)

if(LINUX_DBUS)
  add_sources(PowerComponentDBus.cpp PowerComponentDBus.h)
endif(LINUX_DBUS)

if(WIN32)
  add_sources(PowerComponentWin.cpp PowerComponentWin.h)
endif(WIN32)
