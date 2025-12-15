# CMake script to generate Qt resources (.qrc) file
# Run with: cmake -DOUTPUT=out.qrc -DMAPPINGS="vpath1=rpath1|vpath2=rpath2" -P GenerateQtResources.cmake

if(NOT DEFINED OUTPUT)
  message(FATAL_ERROR "OUTPUT not defined")
endif()

if(NOT DEFINED MAPPINGS)
  message(FATAL_ERROR "MAPPINGS not defined")
endif()

# Convert pipe-delimited string to list
string(REPLACE "|" ";" MAPPINGS_LIST "${MAPPINGS}")

set(qrc_content "<RCC>\n")

foreach(mapping ${MAPPINGS_LIST})
  # Split on first =
  string(FIND "${mapping}" "=" eq_pos)
  if(eq_pos EQUAL -1)
    message(FATAL_ERROR "Invalid mapping '${mapping}', expected virtualpath=realpath")
  endif()

  string(SUBSTRING "${mapping}" 0 ${eq_pos} virtualpath)
  math(EXPR value_start "${eq_pos} + 1")
  string(SUBSTRING "${mapping}" ${value_start} -1 realpath)

  if(IS_DIRECTORY "${realpath}")
    # Recursively add all files from directory
    file(GLOB_RECURSE dir_files RELATIVE "${realpath}" "${realpath}/*")
    foreach(file ${dir_files})
      set(full_virtual "${virtualpath}/${file}")
      # Normalize path
      string(REGEX REPLACE "^/+" "" full_virtual "${full_virtual}")
      string(REGEX REPLACE "/+" "/" full_virtual "${full_virtual}")

      # Split into prefix (directory) and alias (filename)
      get_filename_component(prefix "/${full_virtual}" DIRECTORY)
      get_filename_component(alias "${full_virtual}" NAME)

      string(APPEND qrc_content "<qresource prefix=\"${prefix}\">\n")
      string(APPEND qrc_content " <file alias=\"${alias}\">${realpath}/${file}</file>\n")
      string(APPEND qrc_content "</qresource>\n")
    endforeach()
  else()
    # Single file
    string(REGEX REPLACE "^/+" "" virtualpath_clean "${virtualpath}")
    get_filename_component(prefix "/${virtualpath_clean}" DIRECTORY)
    get_filename_component(alias "${virtualpath_clean}" NAME)

    string(APPEND qrc_content "<qresource prefix=\"${prefix}\">\n")
    string(APPEND qrc_content " <file alias=\"${alias}\">${realpath}</file>\n")
    string(APPEND qrc_content "</qresource>\n")
  endif()
endforeach()

string(APPEND qrc_content "</RCC>")

# Only write if content changed (prevents unnecessary rebuilds)
if(EXISTS "${OUTPUT}")
  file(READ "${OUTPUT}" existing_content)
  if("${existing_content}" STREQUAL "${qrc_content}")
    message(STATUS "resources.qrc unchanged")
    return()
  endif()
endif()

file(WRITE "${OUTPUT}" "${qrc_content}")
message(STATUS "Generated ${OUTPUT}")
