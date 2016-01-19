find_package(PkgConfig)
option(DISABLE_BUNDLED_DEPS "Disable the bundled deps on certain platforms" OFF)

include(FetchDependencies)

if(APPLE AND NOT DISABLE_BUNDLED_DEPS)  
  download_deps(
    "plexmediaplayer-dependencies"
     ARTIFACTNAME konvergo-depends
     DIRECTORY dir
  )
  message("dependencies are: ${dir}")
  set(DEFAULT_ROOT ${dir})
endif(APPLE AND NOT DISABLE_BUNDLED_DEPS)

if(WIN32)
  download_deps(
    "plexmediaplayer-windows-dependencies"
    DIRECTORY dir
    ARTIFACTNAME konvergo-depends-windows
    ARCHSTR mingw32-x86_64
  )
  message("dependencies are: ${dir}")
  set(DEFAULT_ROOT "${dir}")
endif(WIN32)

set(DEPENDENCY_ROOT ${DEFAULT_ROOT} CACHE PATH "Path where the deps are located")

if(IS_DIRECTORY ${DEPENDENCY_ROOT})
  message(STATUS "Going to use bundled deps in directory: ${DEPENDENCY_ROOT}")
  list(APPEND CMAKE_FIND_ROOT_PATH ${DEPENDENCY_ROOT})
  list(APPEND CMAKE_PREFIX_PATH ${DEPENDENCY_ROOT})
  set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_FIND_ROOT_PATH}/lib/pkgconfig)
  set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
  include_directories(${DEPENDENCY_ROOT}/include)
else(IS_DIRECTORY ${DEPENDENCY_ROOT})
  message(STATUS "Not using bundled deps")
endif(IS_DIRECTORY ${DEPENDENCY_ROOT})

find_package(Threads REQUIRED)
find_package(PythonInterp REQUIRED)

# on windows we need to download the updater binary seperately
if(WIN32)
  file(DOWNLOAD https://nightlies.plex.tv/directdl/plex-dependencies/konvergo-qt/updater.exe ${CMAKE_BINARY_DIR}/updater.exe
       SHOW_PROGRESS
       EXPECTED_HASH SHA1=d3b4f70d6542fa42c8edd2b9b93fd0916bf20f07
       TLS_VERIFY OFF)
endif(WIN32)
