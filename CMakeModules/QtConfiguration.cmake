
include(FetchDependencies)

if(WIN32)
  set(WINARCHSTR ARCHSTR windows-x86_64)
endif(WIN32)

if((NOT IS_DIRECTORY ${QTROOT}) AND (NOT "${QTROOT}" STREQUAL ""))
  # Write qt.conf in the Qt depends directory so that the Qt tools can find QML files
  set(QTCONFCONTENT "[Paths]
    Prefix=${QTROOT}
    ")
  file(WRITE ${QTROOT}/bin/qt.conf ${QTCONFCONTENT})
endif()

message(STATUS "Qt root directory: ${QTROOT}")

list(APPEND CMAKE_FIND_ROOT_PATH ${QTROOT})
list(APPEND CMAKE_PREFIX_PATH ${QTROOT})
include_directories(${QTROOT}/include)

set(REQUIRED_QT_VERSION "5.7.0")

set(QTCONFIGROOT ${QTROOT}/lib/cmake/Qt5)
set(components Core Network WebChannel Qml Quick Xml WebEngine Widgets)

if(UNIX AND (NOT APPLE) AND ((NOT BUILD_TARGET STREQUAL "RPI")))
  add_definitions(-DUSE_X11EXTRAS)
  set(components ${components} X11Extras)
endif()

if(LINUX_DBUS)
  set(components ${components} DBus)
endif(LINUX_DBUS)

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

if(QT5_CFLAGS)
	list(REMOVE_DUPLICATES QT5_CFLAGS)
  if(WIN32)
    list(REMOVE_ITEM QT5_CFLAGS -fPIC)
  endif(WIN32)
endif(QT5_CFLAGS)

message(STATUS "Qt version: ${Qt5Core_VERSION_STRING}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT5_CFLAGS}")

set(CMAKE_REQUIRED_INCLUDES ${Qt5WebEngine_INCLUDE_DIRS};${Qt5WebEngine_PRIVATE_INCLUDE_DIRS})
set(CMAKE_REQUIRED_LIBRARIES ${QT5_LIBRARIES})

include(CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES(
"
  #include <QSurfaceFormat>

  int main(int argc, char** argv) {
    QSurfaceFormat::FormatOption o = QSurfaceFormat::UseOptimalOrientation;
    return 0;
  }
" QT5_HAVE_OPTIMALORIENTATION)

if(QT5_HAVE_OPTIMALORIENTATION)
  message(STATUS "QSurfaceFormat::UseOptimalOrientation found")
  add_definitions(-DHAVE_OPTIMALORIENTATION)
endif()
