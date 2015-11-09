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
  find_program(DUMP_SYMS dump_syms HINTS /usr/bin/ ${DEPENDENCY_ROOT}/bin ${DEPENDENCY_ROOT}/lib)
  if(GENERATE_SYMBOLS AND NOT DUMP_SYMS)
    message(WARNING "dump_syms not found")
  endif()
  if(GENERATE_SYMBOLS AND DUMP_SYMS)
    if(APPLE)
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND dsymutil -o ${MAIN_NAME}.dSYM $<TARGET_FILE:${MAIN_TARGET}>
        COMMENT Generating ${MAIN_NAME}.dSYM
        BYPRODUCTS ${MAIN_NAME}.dSYM/Contents/Resources/DWARF/${target} ${MAIN_NAME}.dSYM/Contents/Info.plist
      )
      set(EXTRA_DUMPSYMS_ARGS -g "${CMAKE_CURRENT_BINARY_DIR}/${MAIN_NAME}.dSYM")
    endif(APPLE)

    unset(COMPRESS)
    find_program(COMPRESS_XZ xz HINTS c:/mingw /usr/local/bin)
    find_program(COMPRESS_BZ bzip2 HINTS c:/mingw /usr/local/bin)
    if(COMPRESS_XZ)
      set(COMPRESS_EXT xz)
      set(COMPRESS ${COMPRESS_XZ})
    elseif(COMPRESS_BZ)
      set(COMPRESS_EXT bz2)
      set(COMPRESS ${COMPRESS_BZ})
    endif(COMPRESS_XZ)
    
    set(TARGET_FILE $<TARGET_FILE:${target}>)
    if(WIN32)
      set(TARGET_FILE $<TARGET_PDB_FILE:${target}>)
    endif(WIN32)

    add_custom_command(
      TARGET ${target} POST_BUILD
      BYPRODUCTS ${symfile}.${COMPRESS_EXT}
      COMMAND "${DUMP_SYMS}" ${EXTRA_DUMPSYMS_ARGS} "${TARGET_FILE}" | "${COMPRESS}" > "${symfile}.${COMPRESS_EXT}"
      COMMENT Generating symbols
    )
    install(FILES ${symfile}.${COMPRESS_EXT} DESTINATION ${CMAKE_BINARY_DIR})
  endif(GENERATE_SYMBOLS AND DUMP_SYMS)
endfunction()
