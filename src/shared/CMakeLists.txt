include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(shared STATIC
  Paths.cpp Paths.h
  LocalJsonClient.cpp LocalJsonClient.h
  LocalJsonServer.cpp LocalJsonServer.h
  UniqueApplication.h
  ${CMAKE_BINARY_DIR}/src/Version.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Names.cpp Names.h
)

set_target_properties(shared PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED TRUE)

clang_tidy(shared)
