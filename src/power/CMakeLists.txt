add_sources(PowerComponent.h PowerComponent.cpp)

if(APPLE)
  add_sources(PowerComponentMac.cpp PowerComponentMac.h)
endif(APPLE)

if(USE_X11POWER)
  add_sources(PowerComponentX11.cpp PowerComponentX11.h)
endif(USE_X11POWER)

if(LINUX_DBUS)
  add_sources(PowerComponentDBus.cpp PowerComponentDBus.h)
endif(LINUX_DBUS)

if(WIN32)
  add_sources(PowerComponentWin.cpp PowerComponentWin.h)
endif(WIN32)
