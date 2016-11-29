include_directories(
  ${CMAKE_SOURCE_DIR}/external/qhttp/src
  ${CMAKE_SOURCE_DIR}/external/qslog
  ${CMAKE_SOURCE_DIR}/external/SPMediaKeyTap
  ${CMAKE_SOURCE_DIR}/external/HIDRemote
  ${CMAKE_SOURCE_DIR}/external/plistparser
  ${CMAKE_SOURCE_DIR}/external/letsmove
  ${CMAKE_SOURCE_DIR}/src/player
  core
  shared
)

set(CMAKE_AUTOMOC ON)

add_definitions(-DPREFIX="${CMAKE_INSTALL_PREFIX}")

find_package(Breakpad)

if(BREAKPAD_FOUND)
  include_directories(${BREAKPAD_INCLUDE_DIR})
  set(BREAKPAD_LIBRARIES ${BREAKPAD_LIBRARY})
  if(UNIX)
    # cmake issue: breakpad-client.pc adds this, but cmake discards it
    set(BREAKPAD_LIBRARIES ${BREAKPAD_LIBRARIES} -lpthread)
  endif()
endif()

add_subdirectory(shared)
add_subdirectory(core)
add_subdirectory(display)
add_subdirectory(player)
add_subdirectory(utils)
add_subdirectory(server)
add_subdirectory(ui)
add_subdirectory(input)
add_subdirectory(system)
add_subdirectory(breakpad)
add_subdirectory(settings)
add_subdirectory(power)
add_subdirectory(remote)
add_subdirectory(tools)

get_property(ALL_SRCS GLOBAL PROPERTY SRCS_LIST)
set(MAIN_SRCS main.cpp)

source_group("Source Files" FILES ${MAIN_SRCS})
set(SOURCES ${MAIN_SRCS} ${ALL_SRCS})

# Set some Objective-C flags.
# We need to force the Language to C instead of C++
# and also make sure that we use ARC
#
foreach(S ${SOURCES})
  string(REGEX MATCH ".*\\.m$" MATCH_OBJC ${S})
  if(MATCH_OBJC)
    set_property(SOURCE ${S} PROPERTY COMPILE_FLAGS "-fobjc-arc")
    set_property(SOURCE ${S} PROPERTY LANGUAGE C)
  endif()
endforeach(S SOURCE)

foreach(sfile in ${ALL_SRCS})
  get_filename_component(REALNAME ${sfile} REALPATH)
  get_filename_component(DIRNAME ${REALNAME} DIRECTORY)
  string(REPLACE "${CMAKE_SOURCE_DIR}/src/" "" SUBDIR ${DIRNAME})
  string(TOLOWER ${SUBDIR} SUBDIR)
  string(REPLACE "/" "\\\\" GNAME ${SUBDIR})
  source_group("Source Files\\\\${GNAME}" FILES ${sfile})
endforeach(sfile in ${ALL_SRCS})

file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_SOURCE_DIR}/resources/*)

# generate resources.qrc from directory tree listings
add_custom_command(OUTPUT resources.qrc
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/build-qt-resources.py
          ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc
          /=${CMAKE_SOURCE_DIR}/resources/
          ui/webview.qml=${CMAKE_SOURCE_DIR}/src/ui/webview.qml
  COMMENT "Creating resources.qrc"
  DEPENDS ${CMAKE_SOURCE_DIR}/scripts/build-qt-resources.py
    ${RESOURCE_FILES}
    ${CMAKE_SOURCE_DIR}/src/ui/webview.qml
)
set_source_files_properties(resources.qrc PROPERTIES GENERATED TRUE)
add_custom_command(OUTPUT qrc_resources.cpp
  COMMAND ${Qt5Core_RCC_EXECUTABLE} -name resources -o qrc_resources.cpp resources.qrc
  DEPENDS resources.qrc ${CMAKE_SOURCE_DIR}/src/ui/webview.qml ${RESOURCE_FILES}
)
set_source_files_properties(qrc_resources.cpp PROPERTIES GENERATED TRUE)

list(APPEND RESOURCE_FILES qrc_resources.cpp)

set(MACOSX_BUNDLE_ICON_FILE Plex.icns)
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION_STRING})
set(MACOSX_BUNDLE_BUNDLE_NAME "Plex Media Player")
set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION_STRING})
set(MACOSX_BUNDLE_GUI_IDENTIFIER tv.plex.player)

if(APPLE)
  if(HAVE_UPDATER)
    add_resources(TARGET ${MAIN_TARGET} SOURCES ${UPDATER_PATH} DEST ${INSTALL_RESOURCE_DIR})
  endif(HAVE_UPDATER)

  set_source_files_properties(
    input/apple/HIDRemote/HIDRemote.m
    PROPERTIES COMPILE_FLAGS -fno-objc-arc
  )

  # copy the qt.conf so that when running PMP from a debugger it finds the plugins and QML files
  if(EXISTS ${QTROOT}/bin/qt.conf)
    add_resources(TARGET ${MAIN_TARGET} SOURCES ${QTROOT}/bin/qt.conf DEST ${INSTALL_RESOURCE_DIR})
  endif()


  set(ICON_FILE ${CMAKE_SOURCE_DIR}/bundle/osx/${MACOSX_BUNDLE_ICON_FILE})
  list(APPEND RESOURCE_FILES ${ICON_FILE})
  set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(WIN32)
  list(APPEND RESOURCE_FILES ${CMAKE_SOURCE_DIR}/bundle/win/iconres.rc)
endif()

set(RESOURCE_ROOT .)
if(APPLE)
  set(RESOURCE_ROOT Resources)
endif()

if(NOT DEFINED CONAN_WEB-CLIENT-TV_ROOT)
  message(FATAL_ERROR "Could not find the web-client! Run conan!")
endif()

add_resources(TARGET ${MAIN_TARGET} SOURCES ${CONAN_WEB-CLIENT-TV_ROOT}/tv DEST ${RESOURCE_ROOT}/web-client/tv)
if(DEFINED CONAN_WEB-CLIENT-DESKTOP_ROOT)
  add_resources(TARGET ${MAIN_TARGET} SOURCES ${CONAN_WEB-CLIENT-DESKTOP_ROOT}/desktop DEST ${RESOURCE_ROOT}/web-client/desktop)
endif()

if(NOT APPLE)
  install(FILES ${QTROOT}/resources/qtwebengine_devtools_resources.pak DESTINATION resources)
  install(DIRECTORY ${CONAN_WEB-CLIENT-TV_ROOT}/tv DESTINATION ${INSTALL_RESOURCE_DIR}/web-client)
  if(DEFINED CONAN_WEB-CLIENT-DESKTOP_ROOT)
    install(DIRECTORY ${CONAN_WEB-CLIENT-DESKTOP_ROOT}/desktop DESTINATION ${INSTALL_RESOURCE_DIR}/web-client)
  endif()
endif()

if(XCODE)
  get_resources_source_list(${MAIN_TARGET} XCODE_RESOURCES)
endif()

get_property(BUNDLED_FILES GLOBAL PROPERTY CONFIG_BUNDLED_FILES)
add_executable(${MAIN_TARGET} WIN32 MACOSX_BUNDLE ${SOURCES} ${BUNDLED_FILES} ${RESOURCE_FILES} ${XCODE_RESOURCES})
std_target_properties(${MAIN_TARGET})
set_target_properties(${MAIN_TARGET} PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/osx/Info.plist.in
  INSTALL_RPATH "${QTROOT}/lib"
  OUTPUT_NAME ${MAIN_NAME}
)
copy_resources(${MAIN_TARGET})
clang_tidy(${MAIN_TARGET})

target_link_libraries(${MAIN_TARGET}
  shared
  qhttp
  qslog
  ${MPV_LIBRARY}
  ${OPENGL_LIBS}
  ${QT5_LIBRARIES}
  ${OS_LIBS}
  ${EXTRA_LIBS}
  ${X11_LIBRARIES}
  ${X11_Xrandr_LIB}
  ${BREAKPAD_LIBRARIES}
  ${ICU_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${RPI_LIBS}
)

install(TARGETS ${MAIN_TARGET} DESTINATION ${INSTALL_BIN_DIR})

set(EXE "${MAIN_NAME}.app")
set(LIBPATH ${CMAKE_FIND_ROOT_PATH})
set(SOURCE_ROOT ${CMAKE_SOURCE_DIR})

dumpsyms(${MAIN_TARGET} ${CMAKE_BINARY_DIR}/${MAIN_TARGET}.symbols)

include(CompleteBundle)
