set(INSTALL_BIN_DIR .)
set(INSTALL_RESOURCE_DIR resources)
set(HAVE_UPDATER 1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Oy-")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Oy-")

find_library(WINMM winmm)
find_library(IMMLIB imm32)
find_library(VERLIB version)
find_library(DWMLIB dwmapi)
set(OS_LIBS ${WINMM} ${IMMLIB} ${VERLIB} ${DWMLIB})

set(OS_LIBS ${WINMM} ${IMMLIB} ${VERLIB})
