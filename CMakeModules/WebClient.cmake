include(FetchDependencies)

option(WEB_CLIENT_TV_OLD "" OFF)
option(WEB_CLIENT_DISABLE_DESKTOP "" OFF)

# This is the line to edit when you bump the web-client.
set(WEB_CLIENT_BUILD_ID 142-93fdcea1b12c68)

get_content_of_url(
  URL "https://artifacts.plex.tv/web-client-pmp/${WEB_CLIENT_BUILD_ID}/buildid.cmake"
  CONTENT_VAR BUILDIDS
  FILENAME "buildid-${WEB_CLIENT_BUILD_ID}.cmake"
)

if(WEB_CLIENT_TV_OLD)
  set(TV_VERSION ${TVOLD_VERSION})
endif()

message(STATUS ${BUILDIDS})

include("${DEPENDENCY_CACHE_DIR}/buildid-${WEB_CLIENT_BUILD_ID}.cmake")

if(NOT WEB_CLIENT_DISABLE_DESKTOP)
  download_deps("web-client-desktop"
                BUILD_NUMBER ${WEB_CLIENT_BUILD_ID}
                VARIANT ${DESKTOP_VERSION}
                NO_HASH_FILE
                ARCHSTR "universal"
                BASE_URL "https://artifacts.plex.tv/web-client-pmp/${WEB_CLIENT_BUILD_ID}"
                DIRECTORY WEB_DESKTOP_DIR
  )
endif()

download_deps("web-client-tv"
              BUILD_NUMBER ${WEB_CLIENT_BUILD_ID}
              VARIANT ${TV_VERSION}
              NO_HASH_FILE
              ARCHSTR "universal"
              BASE_URL "https://artifacts.plex.tv/web-client-pmp/${WEB_CLIENT_BUILD_ID}"
              DIRECTORY WEB_TV_DIR
)
