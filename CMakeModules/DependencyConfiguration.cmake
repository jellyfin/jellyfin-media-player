find_package(PkgConfig)
option(DISABLE_BUNDLED_DEPS "Disable the bundled deps on certain platforms" OFF)

if(APPLE AND NOT DISABLE_BUNDLED_DEPS)
  set(DEFAULT_ROOT "${CMAKE_SOURCE_DIR}/dependencies/konvergo-depends-darwin-x86_64-release")
  set(DEPENDENCY_ROOT ${DEFAULT_ROOT} CACHE PATH "Path where the deps are located")
endif(APPLE AND NOT DISABLE_BUNDLED_DEPS)

if(DEPENDENCY_ROOT)
  message(STATUS "Going to use bundled deps in directory: ${DEPENDENCY_ROOT}")
  set(CMAKE_FIND_ROOT_PATH ${DEPENDENCY_ROOT})
  set(CMAKE_PREFIX_PATH ${DEPENDENCY_ROOT})
  set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_FIND_ROOT_PATH}/lib/pkgconfig)
  set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
  include_directories(${CMAKE_FIND_ROOT_PATH}/include)
else(DEPENDENCY_ROOT)
  message(STATUS "Not using bundled deps")
endif(DEPENDENCY_ROOT)

find_package(Threads)

# on windows we need to download the updater binary seperately
if(WIN32)
  file(DOWNLOAD https://nightlies.plex.tv/directdl/plex-dependencies/konvergo-qt/updater.exe ${CMAKE_BINARY_DIR}/updater.exe
       SHOW_PROGRESS
       EXPECTED_HASH SHA1=d3b4f70d6542fa42c8edd2b9b93fd0916bf20f07
       TLS_VERIFY OFF)
endif(WIN32)