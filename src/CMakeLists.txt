include_directories(
  ${CMAKE_SOURCE_DIR}/external/qhttpserver/src
  ${CMAKE_SOURCE_DIR}/external/qslog
  ${CMAKE_SOURCE_DIR}/external/SPMediaKeyTap
  ${CMAKE_SOURCE_DIR}/external/plistparser
  ${CMAKE_SOURCE_DIR}/src/player
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
set(MAIN_SRCS
  Version.h
  ComponentManager.cpp
  ComponentManager.h
  main.cpp
)

# add unix signal management classes
if (UNIX)
  set(MAIN_SRCS ${MAIN_SRCS}
    SignalManager.h
    SignalManager.cpp
  )
endif()

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

find_package(PythonInterp REQUIRED)
file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_SOURCE_DIR}/resources/*)

# generate resources.qrc from directory tree listings
add_custom_command(OUTPUT resources.qrc
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/build-qt-resources.py
          ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc
          /=${CMAKE_SOURCE_DIR}/resources/
          ui/webview.qml=${CMAKE_SOURCE_DIR}/src/ui/webview.qml
          ui/errormessage.qml=${CMAKE_SOURCE_DIR}/src/ui/errormessage.qml
  COMMENT "Creating resources.qrc"
  DEPENDS ${CMAKE_SOURCE_DIR}/scripts/build-qt-resources.py
    ${RESOURCE_FILES}
    ${CMAKE_SOURCE_DIR}/src/ui/webview.qml
    ${CMAKE_SOURCE_DIR}/src/ui/errormessage.qml
)
set_source_files_properties(resources.qrc PROPERTIES GENERATED TRUE)
add_custom_command(OUTPUT qrc_resources.cpp
  COMMAND ${Qt5Core_RCC_EXECUTABLE} -name resources -o qrc_resources.cpp resources.qrc
  DEPENDS resources.qrc ${CMAKE_SOURCE_DIR}/src/ui/webview.qml ${RESOURCE_FILES}
)
set_source_files_properties(qrc_resources.cpp PROPERTIES GENERATED TRUE)

# handle Web-Client resource
include(WebClientResources)

set_source_files_properties(${PROJECT_SOURCE_DIR}/bundle/osx/Plex.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
list(APPEND RESOURCE_FILES qrc_resources.cpp ${WEB_CLIENT_CPP})

if(APPLE)
  list(APPEND RESOURCE_FILES ${PROJECT_SOURCE_DIR}/bundle/osx/Plex.icns)
  if(HAVE_UPDATER)
    list(APPEND RESOURCE_FILES ${UPDATER_PATH})
    set_source_files_properties(${UPDATER_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  endif(HAVE_UPDATER)

  set(PLISTPARSER plistparser)
endif()

set(MACOSX_BUNDLE_ICON_FILE Plex.icns)
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION_STRING})
set(MACOSX_BUNDLE_BUNDLE_NAME "Plex Media Player")
set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION_STRING})
set(MACOSX_BUNDLE_GUI_IDENTIFIER tv.plex.player)

if(WIN32)
  list(APPEND RESOURCE_FILES ${CMAKE_SOURCE_DIR}/bundle/win/iconres.rc)
  install(PROGRAMS ${CMAKE_BINARY_DIR}/updater.exe DESTINATION ${INSTALL_BIN_DIR})
endif()

get_property(BUNDLED_FILES GLOBAL PROPERTY CONFIG_BUNDLED_FILES)
add_executable(${MAIN_TARGET} WIN32 MACOSX_BUNDLE ${SOURCES} ${BUNDLED_FILES} ${RESOURCE_FILES})

set_target_properties(${MAIN_TARGET} PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/bundle/osx/Info.plist.in
  INSTALL_RPATH "${QTROOT}/lib"
  OUTPUT_NAME ${MAIN_NAME}
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(${MAIN_TARGET}
  shared
  qhttpserver
  qslog
  ${PLISTPARSER}
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

if(ENABLE_DUMP_SYMBOLS)
  dumpsyms(${MAIN_TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${MAIN_TARGET}.symbols)
endif()

include(CompleteBundle)
