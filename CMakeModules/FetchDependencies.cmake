include(CMakeParseArguments)

set(DEPENDENCY_CACHE_DIR ${CMAKE_SOURCE_DIR}/Dependencies CACHE PATH "Cache downloaded deps in this directory")
set(DEPENDENCY_UNTAR_DIR ${CMAKE_BINARY_DIR}/dependencies CACHE PATH "Where to untar deps")

if(APPLE)
  set(ARCHSTR "darwin-x86_64")
elseif(WIN32)
  set(OS "windows-i386")
  set(ARCHSTR "windows-i386")
elseif(UNIX)
  set(ARCHSTR ${PLEX_BUILD_TYPE})
endif(APPLE)


function(get_content_of_url)
  set(ARGS URL CONTENT_VAR FILENAME)
  cmake_parse_arguments(CU "" "${ARGS}" "" ${ARGN})
  if(NOT DEFINED CU_FILENAME)
    get_filename_component(CU_FILENAME ${CU_URL} NAME)
  endif(NOT DEFINED CU_FILENAME)

  if(NOT EXISTS ${DEPENDENCY_CACHE_DIR}/${CU_FILENAME})
    message(STATUS "Downloading ${CU_URL} to ${CU_FILENAME}...")

    file(
      DOWNLOAD ${CU_URL} ${DEPENDENCY_CACHE_DIR}/${CU_FILENAME}
      STATUS DL_STATUS
    )

    list(GET DL_STATUS 0 SUCCESS)
    if(NOT SUCCESS EQUAL 0)
      message(FATAL_ERROR "Failed to download ${CU_URL}")
    endif(NOT SUCCESS EQUAL 0)

  endif(NOT EXISTS ${DEPENDENCY_CACHE_DIR}/${CU_FILENAME})

  file(STRINGS ${DEPENDENCY_CACHE_DIR}/${CU_FILENAME} CVAR LIMIT_COUNT 1)
  set(${CU_CONTENT_VAR} ${CVAR} PARENT_SCOPE)
endfunction(get_content_of_url)

function(download_deps DD_NAME)
  set(ARGS DIRECTORY BUILD_NUMBER ARTIFACTNAME VARIANT DEPHASH ARCHSTR)
  cmake_parse_arguments(DD "" "${ARGS}" "" ${ARGN})

  if(NOT DEFINED DD_VARIANT)
    set(DD_VARIANT "release")
  endif(NOT DEFINED DD_VARIANT)

  if(NOT DEFINED DD_ARTIFACTNAME)
    set(DD_ARTIFACTNAME ${DD_NAME})
  endif(NOT DEFINED DD_ARTIFACTNAME)

  if(NOT DEFINED DD_BUILD_NUMBER)
    set(DD_BUILD_NUMBER "latest")
  endif(NOT DEFINED DD_BUILD_NUMBER)

  if(NOT DEFINED DD_ARCHSTR)
    set(DD_ARCHSTR ${ARCHSTR})
  endif(NOT DEFINED DD_ARCHSTR)

  set(BASE_URL "https://nightlies.plex.tv/directdl/plex-dependencies/${DD_NAME}/${DD_BUILD_NUMBER}")
  set(DEP_DIR ${DEPENDENCY_UNTAR_DIR}/${DD_ARCHSTR}-${DD_NAME}/${DD_BUILD_NUMBER}/)

  set(HASH_FILENAME ${DD_NAME}-${DD_BUILD_NUMBER}-hash.txt)
  get_content_of_url(URL ${BASE_URL}/hash.txt CONTENT_VAR DEP_HASH FILENAME ${HASH_FILENAME})

  message(STATUS "Dependency hash is: ${DEP_HASH}")

  set(DEP_DIRNAME "${DD_ARTIFACTNAME}-${DD_ARCHSTR}-${DD_VARIANT}-${DEP_HASH}")
  set(DEP_FILENAME ${DEP_DIRNAME}.tbz2)

  set(${DD_DIRECTORY} ${DEP_DIR}/${DEP_DIRNAME} PARENT_SCOPE)
  set(${DD_DEP_HASH} ${DEP_HASH} PARENT_SCOPE)

  set(DEP_URL "${BASE_URL}/${DEP_FILENAME}")
  get_content_of_url(URL ${DEP_URL}.sha.txt CONTENT_VAR CONTENT_HASH)

  if(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})
    message(STATUS "Clearing out old dependencies ...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEPENDENCY_UNTAR_DIR}/${DD_ARCHSTR}-${DD_NAME})
    file(MAKE_DIRECTORY ${DEP_DIR})

    message(STATUS "Downloading ${DEP_FILENAME}...")

    file(
      DOWNLOAD ${DEP_URL} ${DEPENDENCY_CACHE_DIR}/${DEP_FILENAME}
      SHOW_PROGRESS
      STATUS DEP_STATUS
    )

    list(GET DEP_STATUS 0 DEP_SUCCESS)

    if(NOT DEP_SUCCESS EQUAL 0)
      message(FATAL_ERROR "Failed to download ${DEP_URL}")
    endif()

    message(STATUS "Unpacking ${DEP_FILENAME}...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xjf ${DEPENDENCY_CACHE_DIR}/${DEP_FILENAME}
      WORKING_DIRECTORY ${DEP_DIR}
    )
    if(APPLE)
      message(STATUS "Fixing install library names...")
      execute_process(
        COMMAND ${PROJECT_SOURCE_DIR}/scripts/darwin/fix-install-names.py ${DEP_DIR}/${DEP_DIRNAME}
        WORKING_DIRECTORY ${DEP_DIR}
      )
      message(STATUS "Done")
    endif(APPLE)

    if(EXISTS ${DEP_DIR}/${DEP_DIRNAME}/etc)
      message(STATUS "Removing etc in dependency bundle")
      file(REMOVE_RECURSE ${DEP_DIR}/${DEP_DIRNAME}/etc)
    endif(EXISTS ${DEP_DIR}/${DEP_DIRNAME}/etc)

  else(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})
    message(STATUS "Directory ${DEP_DIR}/${DEP_DIRNAME} already exists, remove it to redownload")
  endif(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})
endfunction(download_deps DD_NAME)
