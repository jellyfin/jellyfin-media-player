
include(FetchDependencies)
if(NOT IS_DIRECTORY ${QTROOT})
  download_deps(
		"plexmediaplayer-qt"
		DIRECTORY dir
		DEPHASH QT_DEPS_HASH
    ARTIFACTNAME konvergo-qt
	)
  set(QTROOT ${dir})
endif()
list(APPEND CMAKE_FIND_ROOT_PATH ${QTROOT})
list(APPEND CMAKE_PREFIX_PATH ${QTROOT})
include_directories(${QTROOT}/include)

# Write qt.conf in the Qt depends directory so that the Qt tools can find QML files
set(QTCONFCONTENT "[Paths]
Prefix=${QTROOT}
")

file(WRITE ${QTROOT}/bin/qt.conf ${QTCONFCONTENT})

set(REQUIRED_QT_VERSION "5.6.0")

message(STATUS ${QTROOT})

set(QTCONFIGROOT ${QTROOT}/lib/cmake/Qt5)
set(components Core Network WebChannel Qml Quick Xml WebEngine)

if(OPENELEC)
  set(components ${components} DBus)
endif(OPENELEC)

foreach(COMP ${components})
	set(mod Qt5${COMP})
	
	# look for the config files in the QtConfigRoot defined above
	set(${mod}_DIR ${QTCONFIGROOT}${COMP})

	# look for the actual package
	find_package(${mod} ${REQUIRED_QT_VERSION} REQUIRED)

	include_directories(${${mod}_INCLUDE_DIRS})
	if(OPENELEC)
		include_directories(${${mod}_PRIVATE_INCLUDE_DIRS})
	endif(OPENELEC)

	list(APPEND QT5_LIBRARIES ${${mod}_LIBRARIES})
	list(APPEND QT5_CFLAGS ${${mod}_EXECUTABLE_COMPILE_FLAGS})
endforeach(COMP ${components})

list(REMOVE_DUPLICATES QT5_CFLAGS)
if(WIN32)
  list(REMOVE_ITEM QT5_CFLAGS -fPIC)
endif(WIN32)

message(STATUS "Qt version: ${Qt5Core_VERSION_STRING}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT5_CFLAGS}")

set(CMAKE_REQUIRED_INCLUDES ${Qt5WebEngine_INCLUDE_DIRS};${Qt5WebEngine_PRIVATE_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${QT5_LIBRARIES})
