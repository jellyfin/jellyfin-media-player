# Generate qrc and run rcc, only write output if changed

# Generate qrc XML
set(qrc "<RCC>\n")

file(GLOB_RECURSE _files "${RESOURCES_DIR}/*")
list(SORT _files)
foreach(_file ${_files})
  file(RELATIVE_PATH _rel "${RESOURCES_DIR}" "${_file}")
  get_filename_component(_prefix "/${_rel}" DIRECTORY)
  get_filename_component(_alias "${_rel}" NAME)
  string(APPEND qrc "<qresource prefix=\"${_prefix}\"><file alias=\"${_alias}\">${_file}</file></qresource>\n")
endforeach()

# webview.qml is now compiled via qt_add_qml_module (AOT)

file(GLOB_RECURSE _files "${NATIVE_DIR}/*")
list(SORT _files)
foreach(_file ${_files})
  file(RELATIVE_PATH _rel "${NATIVE_DIR}" "${_file}")
  string(APPEND qrc "<qresource prefix=\"/web-client/extension\"><file alias=\"${_rel}\">${_file}</file></qresource>\n")
endforeach()

string(APPEND qrc "</RCC>")

# Write qrc to temp file and run rcc
set(_qrc_tmp "${OUTPUT}.qrc")
file(WRITE "${_qrc_tmp}" "${qrc}")

execute_process(
  COMMAND "${RCC}" -name resources "${_qrc_tmp}"
  OUTPUT_VARIABLE _rcc_output
  RESULT_VARIABLE _rcc_result
)

if(NOT _rcc_result EQUAL 0)
  message(FATAL_ERROR "rcc failed")
endif()

# Compare with existing output, write only if changed
if(EXISTS "${OUTPUT}")
  file(READ "${OUTPUT}" _existing)
  if("${_existing}" STREQUAL "${_rcc_output}")
    message(STATUS "Resources unchanged")
    return()
  endif()
endif()

file(WRITE "${OUTPUT}" "${_rcc_output}")
message(STATUS "Resources updated")
