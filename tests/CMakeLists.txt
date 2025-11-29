cmake_minimum_required(VERSION 3.10...3.31)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Test)

add_executable(test_systemcomponent test_systemcomponent.cpp)

target_link_libraries(test_systemcomponent jmp_core Qt6::Test)

target_include_directories(test_systemcomponent PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/shared
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/src/player
)

target_compile_definitions(test_systemcomponent PRIVATE
  PREFIX="${CMAKE_INSTALL_PREFIX}"
)

add_test(NAME test_systemcomponent COMMAND test_systemcomponent)
