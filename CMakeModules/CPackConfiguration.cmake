set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Plex Media Player")
set(CPACK_PACKAGE_VENDOR "Plex")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_NANO})

if(APPLE)
  set(CPACK_SYSTEM_NAME "macosx-x86_64")
elseif(WIN32)
  set(CPACK_SYSTEM_NAME "windows-x86_64")
else()
  set(CPACK_SYSTEM_NAME linux-${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()
set(CPACK_PACKAGE_FILE_NAME "PlexMediaPlayer-${VERSION_STRING}-${CPACK_SYSTEM_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "PlexMediaPlayer-${VERSION_STRING}-src")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "PlexMediaPlayer")
set(CPACK_STRIP_FILES 1)

set(CPACK_GENERATOR "ZIP")

if(WIN32)
	list(APPEND CPACK_GENERATOR "IFW")
endif(WIN32)

# config IFW
set(CPACK_IFW_FRAMEWORK_VERSION 2.0.1)
set(CPACK_IFW_PACKAGE_NAME "Plex Media Player")
set(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY "Plex Media Player")
set(CPACK_IFW_PACKAGE_TITLE "Plex Media Player Installer")
set(CPACK_IFW_PACKAGE_PUBLISHER "Plex")
set(CPACK_IFW_PRODUCT_URL "https://plex.tv")
set(CPACK_IFW_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/bundle/win/Plex.ico)
set(CPACK_IFW_PACKAGE_WINDOW_ICON ${CMAKE_SOURCE_DIR}/resources/images/icon.png)
set(CPACK_IFW_TARGET_DIRECTORY "C:\\Program Files\\Plex Media Player")

if(APPLE)
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
endif(APPLE)

configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/CPackGeneratedConfig.cmake.in ${CMAKE_BINARY_DIR}/CPackGeneratedConfig.cmake)
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_BINARY_DIR}/CPackGeneratedConfig.cmake)

include(CPack)

cpack_add_component(Core DISPLAY_NAME "Plex Media Player" DESCRIPTION "Plex Media Player (Core Application)" REQUIRED)

if(WIN32)
  FILE(TO_CMAKE_PATH ${DEPENDENCY_ROOT} tmp)
  install(FILES ${tmp}/bin/mpv-1.dll DESTINATION .)
  install(FILES ${tmp}/lib/SDL2.dll DESTINATION .)
  install(FILES ${tmp}/lib/libcec.dll DESTINATION .)
  if(IS_DIRECTORY ${CMAKE_BINARY_DIR}/extradlls)
    file(GLOB EXTRADLLS ${CMAKE_BINARY_DIR}/extradlls/*.dll)
    install(FILES ${EXTRADLLS} DESTINATION .)
  endif()
  install(FILES ${CMAKE_SOURCE_DIR}/bundle/win/qt.conf DESTINATION .)
  install(FILES ${CMAKE_SOURCE_DIR}/bundle/win/PlexMediaPlayer-angle.bat DESTINATION .)
  #add_custom_command(TARGET package POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/scripts/WindowsSign.cmd  ${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}.exe WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
  # group/component configuration
  message(STATUS configure IFW)
  include(CPackIFW)
  cpack_ifw_configure_component(Core PRIORITY 1 SCRIPT ${CMAKE_SOURCE_DIR}/bundle/win/shortcut.qs)
endif(WIN32)