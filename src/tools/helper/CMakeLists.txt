include_directories(${PROJECT_SOURCE_DIR}/src)

if(ENABLE_CRASHDUMP)
  set(CRASHDUMP_SRCS CrashUploader.cpp CrashUploader.h)
  add_definitions(-DENABLE_CRASHDUMP -DCRASHDUMP_SECRET="${CRASHDUMP_SECRET}")
endif(ENABLE_CRASHDUMP)

add_executable(${HELPER_TARGET} WIN32
  ${CRASHDUMP_SRCS}
  HelperMain.cpp
  HelperSocket.cpp
  HelperSocket.h
)

target_link_libraries(${HELPER_TARGET} ${Qt5Core_LIBRARIES} ${Qt5Network_LIBRARIES} qslog shared)
set_target_properties(${HELPER_TARGET} PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED TRUE
  OUTPUT_NAME ${HELPER_NAME}
)

if(APPLE)
  set(PLATFORM_RESOURCE_SUFFIX /../Resources)
endif(APPLE)

add_custom_command(
  TARGET ${HELPER_TARGET}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${HELPER_TARGET}> $<TARGET_FILE_DIR:${MAIN_TARGET}>${PLATFORM_RESOURCE_SUFFIX}
)

if(NOT APPLE)
  install(TARGETS ${HELPER_TARGET} DESTINATION ${INSTALL_BIN_DIR})
endif(NOT APPLE)

clang_tidy(${HELPER_TARGET})
