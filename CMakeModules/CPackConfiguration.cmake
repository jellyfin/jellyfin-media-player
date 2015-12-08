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
set(CPACK_IFW_TARGET_DIRECTORY "C:/Program Files/PlexMediaPlayer")

if(APPLE)
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
endif(APPLE)

configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/CPackGeneratedConfig.cmake.in ${CMAKE_BINARY_DIR}/CPackGeneratedConfig.cmake)
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_BINARY_DIR}/CPackGeneratedConfig.cmake)

include(CPack)

cpack_add_component(Core DISPLAY_NAME "Plex Media Player" DESCRIPTION "Plex Media Player (Core Application)" REQUIRED)

# borrowed from https://github.com/peersafe/PeerSafe/blob/master/cmake_modules/package.cmake
if(WIN32)
  if(MSVC)
    if(CMAKE_CL_64)
      set(VC_RUNTIME_DIR "$ENV{VCInstallDir}/redist/x64/Microsoft.VC120.CRT")
    else()
      set(VC_RUNTIME_DIR "$ENV{VCInstallDir}/redist/x86/Microsoft.VC120.CRT")
    endif()
    find_file(MSVCP120 NAMES msvcp120.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
    find_file(MSVCR120 NAMES msvcr120.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
    find_file(VCCORLIB120 NAMES vccorlib120.dll PATHS ${VC_RUNTIME_DIR} NO_DEFAULT_PATH)
    if(NOT MSVCP120)
      set(ERROR_MESSAGE "\nCould not find library msvcp120.dll.\nRun cmake from a Visual Studio Command Prompt.")
      message(FATAL_ERROR "${ERROR_MESSAGE}")
    endif()
  endif()

  install(FILES ${MSVCP120} ${MSVCR120} ${VCCORLIB120} DESTINATION .)
endif()

if(WIN32 AND DEFINED DEPENDENCY_ROOT)
  install(FILES ${CMAKE_SOURCE_DIR}/bundle/win/qt.conf DESTINATION .)
  #add_custom_command(TARGET package POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/scripts/WindowsSign.cmd  ${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}.exe WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
  # group/component configuration
  include(CPackIFW)
  cpack_ifw_configure_component(Core PRIORITY 1 SCRIPT ${CMAKE_SOURCE_DIR}/bundle/win/shortcut.qs)
endif(WIN32 AND DEFINED DEPENDENCY_ROOT)