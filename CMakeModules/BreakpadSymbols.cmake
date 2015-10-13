include(CMakeDependentOption)

set(ENABLE_CRASHDUMP ON)
set(CRASHDUMP_SECRET "" CACHE STRING "Secret for the crashdump uploader")
if (NOT CRASHDUMP_SECRET)
  message(STATUS "Crashdump secret not supplied, disabling crashdump uploading")
  set(ENABLE_CRASHDUMP OFF)
else(NOT CRASHDUMP_SECRET)
  message(STATUS "Enabling crashdump uploader")
endif(NOT CRASHDUMP_SECRET)

cmake_dependent_option(GENERATE_SYMBOLS "Should we generate symbols for binaries?" ON "ENABLE_CRASHDUMP" OFF)

function(dumpsyms target symfile)
  find_program(DUMP_SYMS dump_syms HINTS /usr/bin/ ${DEPENDENCY_ROOT}/bin)
  if(GENERATE_SYMBOLS AND NOT DUMP_SYMS)
    message(STATUS "dump_syms not found")
  endif()
  if(GENERATE_SYMBOLS AND DUMP_SYMS)

    if(APPLE)
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND dsymutil -o ${MAIN_NAME}.dSYM $<TARGET_FILE:${MAIN_TARGET}>
        COMMENT Generating ${MAIN_NAME}.dSYM
        BYPRODUCTS ${MAIN_NAME}.dSYM/Contents/Resources/DWARF/${target} ${MAIN_NAME}.dSYM/Contents/Info.plist
      )
    endif(APPLE)

    unset(COMPRESS)
    find_program(COMPRESS_XZ xz)
    find_program(COMPRESS_BZ bzip2)
    if(COMPRESS_XZ)
      set(COMPRESS_EXT xz)
      set(COMPRESS ${COMPRESS_XZ})
    elseif(COMPRESS_BZ)
      set(COMPRESS_EXT bz2)
      set(COMPRESS ${COMPRESS_BZ})
    endif(COMPRESS_XZ)

    add_custom_command(
      TARGET ${target} POST_BUILD
      BYPRODUCTS ${symfile}.${COMPRESS_EXT}
      COMMAND ${CMAKE_SOURCE_DIR}/scripts/dump-syms.sh "${DUMP_SYMS}" "${COMPRESS}" "$<TARGET_FILE:${target}>" "${symfile}.${COMPRESS_EXT}"
    )
    install(FILES ${symfile}.${COMPRESS_EXT} DESTINATION ${CMAKE_BINARY_DIR})
  endif(GENERATE_SYMBOLS AND DUMP_SYMS)
endfunction()
