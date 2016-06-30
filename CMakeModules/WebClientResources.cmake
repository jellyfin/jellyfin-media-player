include(WebClientVariables)

set(WEB_CLIENT_VERSTR "${WEB_CLIENT_VERSION_NR}-${WEB_CLIENT_VERSION}")
option(SKIP_WEB_CLIENT "Skip downloading the web client" OFF)
set(WEB_CLIENT_FILE plex-web-client-pmp-${WEB_CLIENT_VERSTR}.tbz2)
set(WEB_CLIENT_DIR ${CMAKE_BINARY_DIR}/web-client-${WEB_CLIENT_VERSTR})
message(STATUS "web-client version: ${WEB_CLIENT_VERSTR}")

if(NOT SKIP_WEB_CLIENT)
  set(WEB_CLIENT_URL https://nightlies.plex.tv/directdl/plex-dependencies/plex-web-client-plexmediaplayer/${WEB_CLIENT_BUILDNR}/${WEB_CLIENT_FILE})
  if(NOT EXISTS ${WEB_CLIENT_DIR}/index.html)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/${WEB_CLIENT_FILE})
      safe_download(${WEB_CLIENT_URL}
        FILENAME ${CMAKE_BINARY_DIR}/${WEB_CLIENT_FILE}
        SHOW_PROGRESS
        SHA1 ${WEB_CLIENT_HASH}
      )
    endif()

    file(MAKE_DIRECTORY ${WEB_CLIENT_DIR})
    message(STATUS "Unpacking web-client...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar -xjf ${CMAKE_BINARY_DIR}/${WEB_CLIENT_FILE}
      WORKING_DIRECTORY ${WEB_CLIENT_DIR}
      RESULT_VARIABLE STATUS
    )

    if(NOT STATUS EQUAL 0)
      message(FATAL_ERROR "Failed to unpack web-client")
      file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/${WEB_CLIENT_FILE})
    endif()

    file(REMOVE ${CMAKE_BINARY_DIR}/${WEB_CLIENT_FILE})
  endif()
else(NOT SKIP_WEB_CLIENT)
  message(WARNING "Skipping web-client, you will not a functioning end product")
endif(NOT SKIP_WEB_CLIENT)
