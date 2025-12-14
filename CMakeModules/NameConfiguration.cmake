set(MAIN_TARGET JellyfinDesktop)

# Name of the output binary and data directories
set(MAIN_NAME jellyfin-desktop)
set(DATA_NAME jellyfin-desktop)

if(APPLE)
  set(MAIN_NAME "Jellyfin Desktop")
  set(DATA_NAME "Jellyfin Desktop")
elseif(WIN32)
  set(MAIN_NAME "Jellyfin Desktop")
  set(DATA_NAME "Jellyfin Desktop")
endif(APPLE)

configure_file(src/shared/Names.cpp.in src/shared/Names.cpp @ONLY)