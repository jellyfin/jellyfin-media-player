set(HELPER_TARGET PMPHelper)
set(MAIN_TARGET PlexMediaPlayer)

# Name of the output binary, defaults are only used on Linux
set(HELPER_NAME pmphelper)
set(MAIN_NAME plexmediaplayer)

if(APPLE)
  set(HELPER_NAME "PMP Helper")
  set(MAIN_NAME "Plex Media Player")
elseif(WIN32)
  set(HELPER_NAME "PMPHelper")
  set(MAIN_NAME "PlexMediaPlayer")
endif(APPLE)

configure_file(src/shared/Names.cpp.in src/shared/Names.cpp @ONLY)