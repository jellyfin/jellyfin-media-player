cmake_minimum_required(VERSION 3.10...3.31)

set(CMAKE_AUTOMOC ON)

# Find Qt6 Test module
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS
  Test Core Network Widgets WebChannel Qml Quick Xml
  WebEngineQuick WebEngineCore OpenGL DBus)

# Get all source files that are part of the main application
get_property(ALL_SRCS GLOBAL PROPERTY SRCS_LIST)

# Create test executable - link against all application sources
add_executable(test_systemcomponent
  test_systemcomponent.cpp
  ${ALL_SRCS}
)

if(USE_STATIC_MPVQT)
  set(MPVQT_TARGET MpvQt)
else()
  set(MPVQT_TARGET MpvQt::MpvQt)
endif()

# Link Qt6 libraries and other dependencies
target_link_libraries(test_systemcomponent
  shared
  Qt6::Test
  Qt6::Core
  Qt6::Network
  Qt6::Widgets
  Qt6::WebChannel
  Qt6::Qml
  Qt6::Quick
  Qt6::Xml
  Qt6::WebEngineQuick
  Qt6::WebEngineCore
  Qt6::OpenGL
  Qt6::DBus
  ${MPVQT_TARGET}
  ${QT6_LIBRARIES}
  ${MPV_LIBRARY}
  ${OPENGL_LIBS}
  ${OS_LIBS}
  ${EXTRA_LIBS}
  ${X11_LIBRARIES}
  ${X11_Xrandr_LIB}
  ${ICU_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${RPI_LIBS}
  ${MINIZIP_LIBS}
)

# Include directories - match main application
target_include_directories(test_systemcomponent PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/HIDRemote
  ${CMAKE_SOURCE_DIR}/external/plistparser
  ${CMAKE_SOURCE_DIR}/external/letsmove
  ${CMAKE_SOURCE_DIR}/src/player
)

# Add compile definitions
target_compile_definitions(test_systemcomponent PRIVATE
  PREFIX="${CMAKE_INSTALL_PREFIX}"
)

# Add test to CTest
add_test(NAME test_systemcomponent COMMAND test_systemcomponent)
