project(PlexMediaPlayer CXX C)
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

cmake_policy(SET CMP0020 NEW)
if (POLICY CMP0058)
cmake_policy(SET CMP0058 NEW)
endif()
cmake_policy(SET CMP0017 NEW)

option(OPENELEC "Make an OpenELEC build" OFF)

if (OPENELEC)
  add_definitions(-DKONVERGO_OPENELEC=1)
  Message(STATUS "Making an OpenELEC build")
endif(OPENELEC)

option(BUILD_TARGET "Destination target for the build" "")

if (BUILD_TARGET STREQUAL "RPI")
  add_definitions(-DTARGET_RPI=1)
  set(RPI_LIBS bcm_host)
  Message(STATUS "Build for Raspberry PI target")
endif(BUILD_TARGET STREQUAL "RPI")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME Core)

add_definitions(-DQS_LOG_LINE_NUMBERS -DQHTTPSERVER_EXPORT)

include(DependencyConfiguration)
include(VersionConfiguration)
include(NameConfiguration)
include(utils)
include(QtConfiguration)
include(PlayerConfiguration)
include(InputConfiguration)
include(FindBreakpad)
include(BreakpadSymbols)

if(APPLE)
  include(AppleConfiguration)
elseif(WIN32)
  include(Win32Configuration)
elseif(UNIX AND (NOT APPLE))
  include(LinuxConfiguration)
endif(APPLE)

if(CMAKE_COMPILER_IS_GNUCC)
  include(GCCConfiguration)
endif(CMAKE_COMPILER_IS_GNUCC)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS}")

add_subdirectory(external)
add_subdirectory(src)

include(CPackConfiguration)
