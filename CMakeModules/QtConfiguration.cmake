
include(FetchDependencies)
if(APPLE)
  download_deps("plexmediaplayer-qt" dir)
  set(QTROOT ${dir})
else()
  set(QTROOT "/usr/local/Qt/Qt5.5" CACHE PATH "Root of the QT binaries.")
endif()
set(REQUIRED_QT_VERSION "5.5.0")

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

OPTION(SKIP_QT_TEST "Skip tests for required Qt features" OFF)

if(NOT SKIP_QT_TEST)
	include(CheckCXXSourceCompiles)
        check_cxx_source_compiles("
                #include <private/qquickwebengineview_p.h>
                #include <QColor>
                int main()
                {
                QQuickWebEngineView* view = new QQuickWebEngineView(NULL);
                view->setBackgroundColor(QColor(\"transparent\"));
                }
	" WebEngineBackgroundProperty)

	if(NOT WebEngineBackgroundProperty)
		message(FATAL_ERROR "QQuickWebEngineView doesn't have the background property."
			"This will break video playback. As of Qt 5.5 you need to manually patch and build Qt to get this property."
			"With the release of Qt5.6 it will no longer be required. See qt-patches/README for more details.")
	endif(NOT WebEngineBackgroundProperty)

endif(NOT SKIP_QT_TEST)
