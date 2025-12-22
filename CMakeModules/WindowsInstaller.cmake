find_program(WIX_HEAT heat)
find_program(WIX_CANDLE candle)
find_program(WIX_LIGHT light)
find_program(WIX_INSIGNIA insignia)

get_filename_component(WIX_BIN_DIR ${WIX_CANDLE} DIRECTORY)

configure_file(${PROJECT_SOURCE_DIR}/bundle/win/Version.wxi.in Version.wxi)

set(WIX_VARIABLES -dDeploymentPath="${CMAKE_INSTALL_PREFIX}" -dOutputPath="${CMAKE_INSTALL_PREFIX}" -dBuildPath="${CMAKE_BINARY_DIR}")

include(CMakeParseArguments)

function(wix_harvest_directory dir fragment)
  set(ARGS REFVARIABLE CGROUP DEPENDS)
  cmake_parse_arguments(_WHD "" "${ARGS}" "" ${ARGN})
  
  if(NOT _WHD_REFVARIABLE)
    set(_WHD_REFVARIABLE INSTALLLOCATION)
  endif()
  
  if(_WHD_CGROUP)
    set(CGROUP -cg ${_WHD_CGROUP})
  endif()
  
  add_custom_command(OUTPUT ${fragment}
                     DEPENDS ${_WHD_DEPENDS}
                     COMMAND ${WIX_HEAT} dir "${dir}" -srd -nologo -sw5150 ${CGROUP} -dr ${_WHD_REFVARIABLE} -ag -out ${fragment} -var var.DeploymentPath
                     COMMENT "Harvesting dir ${dir} to ${fragment}")
  add_custom_target(heat_${fragment} DEPENDS ${fragment})
  
endfunction()

function(wix_get_extension VARIABLE)
  foreach(f ${ARGN})
    find_file(${f}_PATH ${f}.dll PATHS ${WIX_BIN_DIR})
    set(EXTS ${EXTS} -ext ${${f}_PATH})
  endforeach()
  
  set(${VARIABLE} ${EXTS} PARENT_SCOPE)
endfunction()

function(wix_candle wsxfile)
  set(ARGS OUTPUT ARCH)
  set(MARGS EXTENSIONS)
  cmake_parse_arguments(_WC "" "${ARGS}" "${MARGS}" ${ARGN})
  
  if(NOT _WC_OUTPUT)
    get_filename_component(WC_BASENAME ${wsxfile} NAME_WE)
    set(_WC_OUTPUT ${WC_BASENAME}.wixobj)
  endif()
  
  if(NOT _WC_ARCH)
    set(_WC_ARCH x64)
  endif()
  
  if(_WC_EXTENSIONS)
    wix_get_extension(WIX_EXTENSIONS ${_WC_EXTENSIONS})
  endif()
  
  add_custom_command(OUTPUT ${_WC_OUTPUT}
                     DEPENDS ${wsxfile}
                     COMMAND ${WIX_CANDLE} -nologo ${WIX_VARIABLES} -I${CMAKE_CURRENT_BINARY_DIR} -arch ${_WC_ARCH} ${wsxfile} ${WIX_EXTENSIONS} -out ${_WC_OUTPUT}
                     COMMENT "Candle file ${_WC_OUTPUT}"
                     )  
endfunction()

function(wix_light)
  set(MARGS OBJS DEPENDS EXTENSIONS)
  set(ARGS OUTPUT BASEDIR TARGET)
  cmake_parse_arguments(_WL "" "${ARGS}" "${MARGS}" ${ARGN})
  
  if(NOT _WL_OUTPUT)
    message(FATAL_ERROR "Need OUTPUT")
  endif()
  
  if(_WL_EXTENSIONS)
    wix_get_extension(WIX_EXTENSIONS ${_WL_EXTENSIONS})
  endif()
  
  if(_WL_BASEDIR)
    set(_WL_BASEDIR -b ${_WL_BASEDIR})
  endif()
  
  if(NOT _WL_TARGET)
    set(_WL_TARGET wix_${_WL_OUTPUT})
  endif()
  
    
  add_custom_command(OUTPUT ${_WL_OUTPUT}
                     DEPENDS ${_WL_OBJS} ${_WL_DEPENDS}
                     COMMAND ${WIX_LIGHT} -nologo ${_WL_BASEDIR} ${_WL_OBJS} ${WIX_VARIABLES} ${WIX_EXTENSIONS} -out ${_WL_OUTPUT}
                     COMMENT "Light it up: ${_WL_OUTPUT}")

                 
  add_custom_target(${_WL_TARGET} DEPENDS ${_WL_OUTPUT})
  
  if(CODE_SIGN)
    if(_WL_OUTPUT MATCHES ".*exe")
      add_custom_command(TARGET ${_WL_TARGET} POST_BUILD
        COMMAND ${WIX_INSIGNIA} -ib ${_WL_OUTPUT} -o engine.exe
        COMMAND ${CMAKE_SOURCE_DIR}/bundle/win/WindowsSign.cmd engine.exe
        COMMAND ${WIX_INSIGNIA} -ab engine.exe ${_WL_OUTPUT} -o ${_WL_OUTPUT}
        COMMENT "Doing insignia dance"
      )
    endif()
    add_custom_command(TARGET ${_WL_TARGET} POST_BUILD
                       COMMAND ${CMAKE_SOURCE_DIR}/bundle/win/WindowsSign.cmd ${_WL_OUTPUT}
                       COMMENT Signing ${_WL_OUTPUT}
    )
  endif()
endfunction()

function(wix_create_installer output)
  set(MARGS WXS_FILES EXTENSIONS DEPENDS)
  set(ARGS ARCH BASEDIR TARGET)
  cmake_parse_arguments(_WCI "" "${ARGS}" "${MARGS}" ${ARGN})
  
  foreach(wxs ${_WCI_WXS_FILES})
    get_filename_component(WXSBASE ${wxs} NAME_WE)
    wix_candle(${wxs} OUTPUT ${WXSBASE}.wixobj EXTENSIONS ${_WCI_EXTENSIONS} ARCH ${_WCI_ARCH})
    list(APPEND WIXOBJS ${WXSBASE}.wixobj)
  endforeach()
  
  wix_light(OBJS ${WIXOBJS}
            EXTENSIONS ${_WCI_EXTENSIONS}
            OUTPUT ${output}
            DEPENDS ${_WCI_DEPENDS}
            BASEDIR ${_WCI_BASEDIR}
            TARGET ${_WCI_TARGET})
endfunction()

add_custom_target(wix_install
                  COMMAND ${CMAKE_COMMAND} -P cmake_install.cmake
                  COMMENT "Copying files..."
                  DEPENDS JellyfinDesktop)

wix_harvest_directory("${CMAKE_INSTALL_PREFIX}" files.wxs CGROUP ProgramFilesComponentGroup DEPENDS wix_install)

wix_create_installer(PMP.msi 
                     WXS_FILES files.wxs "${PROJECT_SOURCE_DIR}/bundle/win/JMP.wxs"
                     EXTENSIONS WixUtilExtension WixFirewallExtension
                     BASEDIR "${PROJECT_SOURCE_DIR}/bundle/win"
)

if (CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(INSTALLER_ARCH_STR windows-x64)
else()
  set(INSTALLER_ARCH_STR windows-x86)
endif()

wix_create_installer(JellyfinDesktop-${VERSION_STRING}-${INSTALLER_ARCH_STR}.exe
                     TARGET JellyfinDesktopInstaller
                     WXS_FILES "${PROJECT_SOURCE_DIR}/bundle/win/Bundle.wxs"
                     EXTENSIONS WixUtilExtension WixBalExtension
                     DEPENDS wix_PMP.msi
                     BASEDIR "${PROJECT_SOURCE_DIR}/bundle/win"
)

add_custom_target(windows_package DEPENDS JellyfinDesktopInstaller)
