add_sources(TaskbarComponent.h TaskbarComponent.cpp)

if(WIN32)
  add_sources(TaskbarComponentWin.cpp TaskbarComponentWin.h)
endif(WIN32)
