include(WebClientVariables)

option(SKIP_WEB_CLIENT "Skip downloading the web client" OFF)

if(NOT SKIP_WEB_CLIENT)
  set(WEB_CLIENT_CPP plex-web-client-konvergo-${WEB_CLIENT_VERSION}.cpp)
  set(WEB_CLIENT_URL https://nightlies.plex.tv/directdl/plex-dependencies/plex-web-client-plexmediaplayer/${WEB_CLIENT_BUILDNR}/plex-web-client-konvergo-${WEB_CLIENT_VERSION}.cpp.tbz2)

  message(STATUS "web-client version: ${WEB_CLIENT_VERSION}")

  set(LOCAL_WEB_CLIENT false)
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2")
    file(SHA1 "${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2" EXISTING_HASH)
    if("${EXISTING_HASH}" STREQUAL "${WEB_CLIENT_HASH}")
        set(LOCAL_WEB_CLIENT true)
    endif("${EXISTING_HASH}" STREQUAL "${WEB_CLIENT_HASH}")
  endif(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2")

  if(NOT LOCAL_WEB_CLIENT)
    file(
      DOWNLOAD ${WEB_CLIENT_URL} ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2
      EXPECTED_HASH SHA1=${WEB_CLIENT_HASH}
      INACTIVITY_TIMEOUT 20
      TIMEOUT 3600
      SHOW_PROGRESS
    )
  endif(NOT LOCAL_WEB_CLIENT)

  add_custom_command(
    OUTPUT ${WEB_CLIENT_CPP}
    COMMAND ${CMAKE_COMMAND} -E tar xjf ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2
    COMMENT "Unpacking: ${WEB_CLIENT_CPP}.tbz2"
  )

  add_custom_target(UnpackWebClientResource
    DEPENDS ${WEB_CLIENT_CPP}
  )
else(NOT SKIP_WEB_CLIENT)
  message(WARNING "Skipping web-client, you will not a functioning end product")
endif(NOT SKIP_WEB_CLIENT)
