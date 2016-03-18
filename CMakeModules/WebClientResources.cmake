include(WebClientVariables)

option(SKIP_WEB_CLIENT "Skip downloading the web client" OFF)

if(NOT SKIP_WEB_CLIENT)
  set(WEB_CLIENT_CPP plex-web-client-konvergo-${WEB_CLIENT_VERSION}.cpp)
  set(WEB_CLIENT_URL https://nightlies.plex.tv/directdl/plex-dependencies/plex-web-client-plexmediaplayer/${WEB_CLIENT_BUILDNR}/plex-web-client-konvergo-${WEB_CLIENT_VERSION}.cpp.tbz2)

  message(STATUS "web-client version: ${WEB_CLIENT_VERSION}")

  file(
    DOWNLOAD ${WEB_CLIENT_URL} ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.tbz2
    EXPECTED_HASH SHA1=${WEB_CLIENT_HASH}
    TIMEOUT 100
    SHOW_PROGRESS
    TLS_VERIFY ON
  )

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
