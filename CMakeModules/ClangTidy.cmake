get_filename_component(CXX_COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY CACHE)
find_program(CLANG_TIDY clang-tidy NAMES clang-tidy clang-tidy-3.9 clang-tidy-3.8 clang-tidy-3.7 HINTS ${CXX_COMPILER_DIR})
find_program(CLANG_REPLACE clang-apply-replacements NAMES clang-apply-replacements clang-apply-replacements-3.9 clang-apply-replacements-3.8 clang-apply-replacements-3.7 HINTS ${CXX_COMPILER_DIR})

if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT CLANG_TIDY STREQUAL CLANG_TIDY-NOTFOUND)
  macro(clang_tidy target)
    get_target_property(TARGET_SOURCES ${target} SOURCES)
    get_target_property(TARGET_SOURCE_DIR ${target} SOURCE_DIR)
    get_target_property(TARGET_BINARY_DIR ${target} BINARY_DIR)
    
    foreach(s ${TARGET_SOURCES})
      get_filename_component(FILE_PATH ${s} ABSOLUTE BASE_DIR ${TARGET_SOURCE_DIR})
      get_filename_component(FILE_NAME ${FILE_PATH} NAME)
      get_source_file_property(FILE_LANG ${FILE_PATH} LANGUAGE)
      get_source_file_property(FILE_GENERATED ${FILE_PATH} GENERATED)

      if("${FILE_LANG}" STREQUAL "CXX" OR "${FILE_LANG}" STREQUAL "C" AND NOT FILE_GENERATED)
        add_custom_command(OUTPUT ${TARGET_BINARY_DIR}/_tidy/${FILE_NAME}
                           COMMAND ${CLANG_TIDY} -p "${CMAKE_BINARY_DIR}" ${FILE_PATH}
                           COMMENT "clang-tidy ${FILE_NAME} ..."
                           WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                          )
        list(APPEND TIDY_TARGETS ${TARGET_BINARY_DIR}/_tidy/${FILE_NAME})
      endif()
    endforeach()
    add_custom_target(${target}_tidy DEPENDS ${TIDY_TARGETS})
  endmacro()  
else()
  macro(clang_tidy target)
    message(STATUS "clang-tidy not enabled, pass -DCMAKE_EXPORT_COMPILE_COMMANDS=on to cmake to enable it")
  endmacro()
endif()

add_custom_target(tidy_all DEPENDS ${MAIN_TARGET}_tidy ${HELPER_TARGET}_tidy shared_tidy)
