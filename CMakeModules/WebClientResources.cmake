include(WebClientVariables)

option(SKIP_WEB_CLIENT "Skip downloading the web client" OFF)

if(NOT SKIP_WEB_CLIENT)
  set(WEB_CLIENT_CPP web-client-${WEB_CLIENT_VERSION}.cpp)
  set(WEB_CLIENT_URL https://nightlies.plex.tv/directdl/plex-web-client-plexmediaplayer/master/plex-web-client-konvergo-${WEB_CLIENT_VERSION}.cpp.bz2)

  message(STATUS "web-client version: ${WEB_CLIENT_VERSION}")

  file(
    DOWNLOAD ${WEB_CLIENT_URL} ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.bz2
    EXPECTED_HASH SHA1=${WEB_CLIENT_HASH}
    TIMEOUT 100
    SHOW_PROGRESS
    TLS_VERIFY ON
  )

  find_program(BUNZIP2 bunzip2${CMAKE_EXECUTABLE_SUFFIX})
  if(${BUNZIP2} MATCHES NOT_FOUND)
    message(FATAL_ERROR "Can't fid bunzip2")
  endif(${BUNZIP2} MATCHES NOT_FOUND)

  add_custom_command(
    OUTPUT ${WEB_CLIENT_CPP}
    COMMAND ${BUNZIP2} -k -f ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.bz2
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${WEB_CLIENT_CPP}.bz2
    COMMENT "Unpacking: ${WEB_CLIENT_CPP}.bz2"
  )

  add_custom_target(UnpackWebClientResource
    DEPENDS ${WEB_CLIENT_CPP}
  )
else(NOT SKIP_WEB_CLIENT)
  message(WARNING "Skipping web-client, you will not a functioning end product")
endif(NOT SKIP_WEB_CLIENT)
