if(CMAKE_EXPORT_COMPILE_COMMANDS)
  find_program(CLANG_TIDY clang-tidy NAMES clang-tidy-3.9 clang-tidy-3.8 clang-tidy-3.7)
  find_program(CLANG_REPLACE clang-apply-replacements NAMES clang-apply-replacements-3.9 clang-apply-replacements-3.8 clang-apply-replacements-3.7)
  if(NOT CLANG_TIDY STREQUAL CLANG_TIDY-NOTFOUND)

    set(CLANG_TIDY_COMMAND ${PROJECT_SOURCE_DIR}/scripts/run-clang-tidy.py -clang-apply-replacements-binary ${CLANG_REPLACE} -clang-tidy-binary ${CLANG_TIDY} -header-filter=${PROJECT_SOURCE_DIR}/src/.*)

    add_custom_target(tidy
      COMMAND ${CLANG_TIDY_COMMAND} ${PROJECT_SOURCE_DIR}/src
      USES_TERMINAL
    )

    add_custom_target(tidy-fix
      COMMAND ${CLANG_TIDY_COMMAND} -fix ${PROJECT_SOURCE_DIR}/src
      USES_TERMINAL
    )

  endif()
endif()
