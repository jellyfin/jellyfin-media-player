include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(socket-client
  SocketClient.cpp
  $<TARGET_OBJECTS:shared>
)
set_target_properties(socket-client PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED TRUE
)
target_link_libraries(socket-client ${Qt5Network_LIBRARIES} ${Qt5Core_LIBRARIES} qslog)