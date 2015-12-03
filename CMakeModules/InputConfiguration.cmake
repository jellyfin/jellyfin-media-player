message(STATUS ${CMAKE_FIND_ROOT_PATH})
OPTION(ENABLE_SDL2 "Enable SDL2 for joystick handling" ON)
if(ENABLE_SDL2)
  find_package(SDL2)
  if(SDL2_FOUND)
    list(APPEND ENABLED_INPUTS SDL2)

    if(NOT WIN32)
      find_package(Iconv)

      if(NOT ICONV_FOUND)
        unset(SDL2_FOUND)
      endif(NOT ICONV_FOUND)

      find_package(DL)

      if(NOT DL_FOUND)
        unset(SDL2_FOUND)
      endif(NOT DL_FOUND)

      list(APPEND SDL2_LIBRARY ${ICONV_LIBRARIES} ${DL_LIBRARIES})
    endif()

    if(APPLE)
      find_package(Iconv)

      if(NOT ICONV_FOUND)
        unset(SDL2_FOUND)
      endif(NOT ICONV_FOUND)

      list(APPEND SDL2_LIBRARY ${ICONV_LIBRARIES})
      find_library(FORCEFEEDBACK ForceFeedback)
      find_library(CARBON Carbon)
      list(APPEND SDL2_LIBRARY ${FORCEFEEDBACK} ${CARBON})
    endif(APPLE)

    if(SDL2_FOUND)
      add_definitions(-DHAVE_SDL)
      include_directories(${SDL2_INCLUDE_DIR})
      set(EXTRA_LIBS ${SDL2_LIBRARY})
    endif(SDL2_FOUND)

  endif(SDL2_FOUND)

endif(ENABLE_SDL2)

OPTION(ENABLE_CEC "Enable HDMI/CEC support with libCEC" ON)
if(ENABLE_CEC)
  find_package(CEC)
  if(CEC_FOUND)
    list(APPEND ENABLED_INPUTS CEC)
    add_definitions(-DHAVE_CEC)
    include_directories(${CEC_INCLUDE_DIR})
    set(EXTRA_LIBS ${EXTRA_LIBS} ${CEC_LIBRARY})
  endif(CEC_FOUND)

endif(ENABLE_CEC)

if(UNIX AND NOT APPLE)
  OPTION(ENABLE_LIRC "Enable LIRC for Linux IR handling" ON)
  if(ENABLE_LIRC)
    list(APPEND ENABLED_INPUTS LIRC)
    add_definitions(-DHAVE_LIRC)
  endif(ENABLE_LIRC)
endif(UNIX AND NOT APPLE)

if(APPLE)
  list(APPEND ENABLED_INPUTS "AppleRemote")
endif(APPLE)

string(REPLACE ";" " " _STR "${ENABLED_INPUTS}")
message(STATUS "Enabled Inputs: " ${_STR})
