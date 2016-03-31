# MSVC goes totally bananas if we pass it -Wall
if(NOT MSVC)
  enable_if_supported(COMPILER_FLAGS "-Wall")
endif()

enable_if_supported(COMPILER_FLAGS "-fno-omit-frame-pointer")
enable_if_supported(COMPILER_FLAGS "-mmacosx-version-min=10.9")
enable_if_supported(COMPILER_FLAGS "/Oy-")

enable_if_links(LINK_FLAGS "-flto")
enable_if_links(LINK_FLAGS "-fuse-ld=gold")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")
