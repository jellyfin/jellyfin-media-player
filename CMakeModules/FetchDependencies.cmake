set(DEP_DIR ${CMAKE_BINARY_DIR}/dependencies)

if(APPLE)
  set(OS "darwin")
elseif(WIN32)
  set(OS "windows")
elseif(UNIX)
  set(OS "linux")
endif()

# note that hardcoding the ARCH here is not correct.
set(ARCHSTR "${OS}-x86_64")

function(download_deps depname dirpath)
  file(MAKE_DIRECTORY ${DEP_DIR})

  message(STATUS "Downloading ${depname}.hash.txt...")
  file(
    DOWNLOAD "https://nightlies.plex.tv/directdl/plex-dependencies/${depname}/latest/hash.txt" ${DEP_DIR}/${depname}-hash.txt
    STATUS HASH_STATUS
  )
  list(GET HASH_STATUS 0 SUCCESS)

  if(SUCCESS EQUAL 0)
    file(STRINGS ${DEP_DIR}/${depname}-hash.txt DEP_HASH LIMIT_COUNT 1)

    if(depname STREQUAL plexmediaplayer-qt)
      set(DEP_DIRNAME "konvergo-qt-${ARCHSTR}-release-${DEP_HASH}")
    elseif(depname STREQUAL plexmediaplayer-dependencies)
      set(DEP_DIRNAME "konvergo-depends-${ARCHSTR}-release-${DEP_HASH}")
    elseif(depname STREQUAL plexmediaplayer-windows-dependencies)
      set(ARCHSTR "mingw32-x86_64")
      set(DEP_DIRNAME "konvergo-depends-windows-${ARCHSTR}-release-${DEP_HASH}")
    else()
      set(DEP_DIRNAME "${depname}-${ARCHSTR}-release-${DEP_HASH}")
    endif()

    set(DEP_FILENAME ${DEP_DIRNAME}.tbz2)
    set(DEP_URL "https://nightlies.plex.tv/directdl/plex-dependencies/${depname}/latest/${DEP_FILENAME}")

    set(${dirpath} ${DEP_DIR}/${DEP_DIRNAME} PARENT_SCOPE)

    if(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})
      message(STATUS "Downloading ${DEP_FILENAME}.sha.txt...")
      file(DOWNLOAD ${DEP_URL}.sha.txt ${DEP_DIR}/${DEP_FILENAME}.sha.txt STATUS SHA_STATUS)

      list(GET SHA_STATUS 0 SHASUCCESS)

      if(SHASUCCESS EQUAL 0)
        file(STRINGS ${DEP_DIR}/${DEP_FILENAME}.sha.txt CONTENT_HASH_RAW LIMIT_COUNT 1)
        string(SUBSTRING ${CONTENT_HASH_RAW} 0 40 CONTENT_HASH)

        message(STATUS "Downloading ${DEP_FILENAME}...")

        file(
          DOWNLOAD ${DEP_URL} ${DEP_DIR}/${DEP_FILENAME}
          EXPECTED_HASH SHA1=${CONTENT_HASH}
          SHOW_PROGRESS
          STATUS DEP_STATUS
        )

        list(GET DEP_STATUS 0 DEP_SUCCESS)

        if(NOT DEP_SUCCESS EQUAL 0)
          message(FATAL_ERROR "Failed to download ${DEP_URL}")
        endif()

        if(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})
          message(STATUS "Unpacking ${DEP_FILENAME}...")
          execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xjf ${DEP_DIR}/${DEP_FILENAME}
            WORKING_DIRECTORY ${DEP_DIR}
          )
          if(APPLE)
            message(STATUS "Fixing install library names...")
            execute_process(
              COMMAND ${PROJECT_SOURCE_DIR}/scripts/fix-install-names.py ${DEP_DIR}/${DEP_DIRNAME}
              WORKING_DIRECTORY ${DEP_DIR}
            )
            message(STATUS "Done")
          endif(APPLE)
          if(WIN32 AND EXISTS ${DEP_DIR}/${DEP_DIRNAME}/bin/mpv-1.def)
            message(STATUS "Fixing mpv.lib...")
            execute_process(
              COMMAND LIB /def:bin\\mpv-1.def /out:lib\\mpv.lib /MACHINE:X64
              WORKING_DIRECTORY ${DEP_DIR}/${DEP_DIRNAME}
            )
          endif(WIN32 AND EXISTS ${DEP_DIR}/${DEP_DIRNAME}/bin/mpv-1.def)
        endif()
      else(SHASUCCESS EQUAL 0)
        list(GET SHA_STATUS 1 SHAERROR)
        message(FATAL_ERROR "Failed to download ${DEP_FILENAME}.sha.txt error: ${SHAERROR}")
      endif(SHASUCCESS EQUAL 0)
  else(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})
    message(STATUS "Directory ${DEP_DIR}/${DEP_DIRNAME} already exists, remove it to redownload")
  endif(NOT EXISTS ${DEP_DIR}/${DEP_DIRNAME})

  else(SUCCESS EQUAL 0)
    list(GET HASH_STATUS 1 HASHERROR)
    message(FATAL_ERROR "Failed to download ${depname}.hash.txt error: ${HASHERROR}")
  endif(SUCCESS EQUAL 0)    
endfunction(download_deps depname)