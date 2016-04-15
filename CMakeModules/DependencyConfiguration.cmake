find_package(PkgConfig)
option(DISABLE_BUNDLED_DEPS "Disable the bundled deps on certain platforms" OFF)

include(FetchDependencies)

if(NOT DISABLE_BUNDLED_DEPS)
  if(OPENELEC)
    set(DEPENDCY_FOLDER plexmediaplayer-openelec-codecs)
  elseif(APPLE OR WIN32)
    set(DEPENDCY_FOLDER plexmediaplayer-dependencies-codecs)
  endif()
  download_deps(
    "${DEPENDCY_FOLDER}"
    ARTIFACTNAME konvergo-codecs-depends
    DIRECTORY dir
    DEPHASH_VAR DEPS_HASH
    DYLIB_SCRIPT_PATH ${PROJECT_SOURCE_DIR}/scripts/fix-install-names.py
  )
  message("dependencies are: ${dir}")
  set(DEFAULT_ROOT ${dir})
endif(NOT DISABLE_BUNDLED_DEPS)

if(WIN32)
  if(NOT EXISTS ${dir}/lib/mpv.lib)
    if(ARCHSTR STREQUAL "windows-x86_64")
      set(ENV{PMP_VC_ARCH} "amd64")
      set(ENV{PMP_LIB_ARCH} "X64")
    else()
      set(ENV{PMP_VC_ARCH} "x86")
      set(ENV{PMP_LIB_ARCH} "x86")
    endif()
    execute_process(
      COMMAND ${PROJECT_SOURCE_DIR}/scripts/make_mpv_lib.bat
      WORKING_DIRECTORY ${dir}
    )
  endif(NOT EXISTS ${dir}/lib/mpv.lib)
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
