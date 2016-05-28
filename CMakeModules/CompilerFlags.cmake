# MSVC goes totally bananas if we pass it -Wall
if(NOT MSVC)
  enable_if_supported(COMPILER_FLAGS "-Wall")
endif()

enable_if_supported(COMPILER_FLAGS "-Wshorten-64-to-32")
enable_if_supported(COMPILER_FLAGS "-fno-omit-frame-pointer")
enable_if_supported(COMPILER_FLAGS "-mmacosx-version-min=10.9")
enable_if_supported(COMPILER_FLAGS "/Oy-")

# Flags only for external libs
enable_if_supported(COMPILER_FLAGS_THIRD_PARTY "-Wno-shorten-64-to-32")
enable_if_supported(COMPILER_FLAGS_THIRD_PARTY "/wd4244")
enable_if_supported(COMPILER_FLAGS_THIRD_PARTY "/wd4267")

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
if (NOT (build_type_lower MATCHES "debug"))
  enable_if_links(LINK_FLAGS "-flto")
endif()

enable_if_links(LINK_FLAGS "-fuse-ld=gold")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_FLAGS}")
