find_package(PkgConfig)

include(FetchDependencies)

if(DEPENDENCY_TOKEN)
  set(DEPENDCY_FOLDER "")
  if(OPENELEC)
    set(DEPENDCY_FOLDER plexmediaplayer-openelec-codecs)
    set(DEPS_BUILD_NUMBER 61)
  elseif(APPLE OR WIN32)
    set(DEPENDCY_FOLDER plexmediaplayer-dependencies-codecs)
    set(DEPS_BUILD_NUMBER 194)
  endif()
  if(NOT (DEPENDCY_FOLDER STREQUAL ""))
    download_deps(
      "${DEPENDCY_FOLDER}"
      ARTIFACTNAME konvergo-codecs-depends
      BUILD_NUMBER ${DEPS_BUILD_NUMBER}
      DIRECTORY dir
      DEPHASH_VAR DEPS_HASH
      DYLIB_SCRIPT_PATH ${PROJECT_SOURCE_DIR}/scripts/fix-install-names.py
      TOKEN ${DEPENDENCY_TOKEN}
    )
    message("dependencies are: ${dir}")
    set(DEFAULT_ROOT ${dir})
  endif()
endif(DEPENDENCY_TOKEN)

if(WIN32)
  message("dependencies are: ${dir}")
  set(DEFAULT_ROOT "${dir}")

  download_deps(
    "windows-redist"
    DIRECTORY VCREDIST_DIR
    ARTIFACTNAME windows-redist-2015
    ARCHSTR windows-x86_x64
  )
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
