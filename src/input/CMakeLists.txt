set(INPUT_SRCS
  InputComponent.cpp
  InputComponent.h
  InputMapping.cpp
  InputMapping.h
  InputKeyboard.h
  InputSocket.h
  InputSocket.cpp
)

if(APPLE)
  add_subdirectory(apple)
endif(APPLE)

if(SDL2_FOUND)
  list(APPEND INPUT_SRCS InputSDL.cpp InputSDL.h)
endif(SDL2_FOUND)

if(ENABLE_LIRC)
  list(APPEND INPUT_SRCS InputLIRC.cpp InputLIRC.h)
endif(ENABLE_LIRC)

if(CEC_FOUND)
  list(APPEND INPUT_SRCS InputCEC.cpp InputCEC.h)
endif(CEC_FOUND)

add_sources(${INPUT_SRCS})

