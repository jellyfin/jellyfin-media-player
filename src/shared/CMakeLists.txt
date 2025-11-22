include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(shared STATIC
  Paths.cpp Paths.h
  LocalJsonClient.cpp LocalJsonClient.h
  LocalJsonServer.cpp LocalJsonServer.h
  UniqueApplication.h
  ${CMAKE_BINARY_DIR}/src/core/Version.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Names.cpp Names.h
)

# GuiPrivate only needed for RPI builds
if(BUILD_TARGET STREQUAL "RPI")
  find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
endif()

target_link_libraries(shared
Qt6::Core
Qt6::Network
Qt6::Qml
Qt6::WebChannel
Qt6::Gui
)

if(BUILD_TARGET STREQUAL "RPI")
  target_link_libraries(shared Qt6::GuiPrivate)
endif()
std_target_properties(shared)

clang_tidy(shared)
