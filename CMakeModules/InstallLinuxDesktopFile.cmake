install (FILES ${CMAKE_SOURCE_DIR}/resources/meta/org.jellyfin.JellyfinDesktop.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications)
install (FILES ${CMAKE_SOURCE_DIR}/resources/meta/org.jellyfin.JellyfinDesktop.appdata.xml DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/metainfo)
install (FILES ${CMAKE_SOURCE_DIR}/resources/images/icon.svg DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/scalable/apps RENAME org.jellyfin.JellyfinDesktop.svg)
