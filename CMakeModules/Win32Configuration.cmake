set(INSTALL_BIN_DIR .)
set(INSTALL_RESOURCE_DIR resources)
set(HAVE_UPDATER 1)

if(${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Oy-")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Oy-")
endif()

find_library(WINMM winmm)
find_library(IMMLIB imm32)
find_library(VERLIB version)
find_library(DWMLIB dwmapi)
find_library(AVRTLIB avrt)
set(OS_LIBS ${WINMM} ${IMMLIB} ${VERLIB} ${DWMLIB} ${AVRTLIB})
