install (FILES ${CMAKE_SOURCE_DIR}/resources/meta/com.github.iwalton3.jellyfin-media-player.desktop DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications)
install (FILES ${CMAKE_SOURCE_DIR}/resources/meta/com.github.iwalton3.jellyfin-media-player.appdata.xml DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/metainfo)
install (FILES ${CMAKE_SOURCE_DIR}/resources/images/icon.svg DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/icons/hicolor/scalable/apps RENAME com.github.iwalton3.jellyfin-media-player.svg)
