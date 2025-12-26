# Find Inno Setup Compiler
find_program(ISCC iscc
  PATHS
    "$ENV{LOCALAPPDATA}/Programs/Inno Setup 6"
    "$ENV{ProgramFiles\(x86\)}/Inno Setup 6"
    "$ENV{ProgramFiles}/Inno Setup 6"
    "C:/Program Files (x86)/Inno Setup 6"
    "C:/Program Files/Inno Setup 6"
  DOC "Inno Setup Compiler"
)

if(NOT ISCC)
  message(WARNING "Inno Setup Compiler (iscc) not found. Windows installer will not be built.")
  return()
endif()

message(STATUS "Found Inno Setup: ${ISCC}")

# Configure the Inno Setup script with version and paths
configure_file(
  ${PROJECT_SOURCE_DIR}/bundle/win/JellyfinDesktop.iss.in
  ${CMAKE_CURRENT_BINARY_DIR}/JellyfinDesktop.iss
  @ONLY
)

# Create install target that copies files to CMAKE_INSTALL_PREFIX
add_custom_target(innosetup_install
  COMMAND ${CMAKE_COMMAND} -P cmake_install.cmake
  COMMENT "Copying files for installer..."
  DEPENDS JellyfinDesktop
)

# Determine architecture string for output filename
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(INSTALLER_ARCH_STR windows-x64)
else()
  set(INSTALLER_ARCH_STR windows-x86)
endif()

set(INSTALLER_BASE_NAME "JellyfinDesktop-${VERSION_STRING}-${INSTALLER_ARCH_STR}")
set(INSTALLER_OUTPUT_NAME "${INSTALLER_BASE_NAME}.exe")

# Create the installer using Inno Setup
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${INSTALLER_OUTPUT_NAME}
  DEPENDS innosetup_install ${CMAKE_CURRENT_BINARY_DIR}/JellyfinDesktop.iss
  COMMAND ${ISCC}
    /O${CMAKE_CURRENT_BINARY_DIR}
    /F${INSTALLER_BASE_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}/JellyfinDesktop.iss
  COMMENT "Building Inno Setup installer: ${INSTALLER_OUTPUT_NAME}"
  VERBATIM
)

# Target to build the installer
add_custom_target(JellyfinDesktopInstaller
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${INSTALLER_OUTPUT_NAME}
)

# Alias target for convenience
add_custom_target(windows_package DEPENDS JellyfinDesktopInstaller)

# Create portable ZIP archive (includes bundled runtime DLLs in root)
set(ZIP_OUTPUT_NAME "${INSTALLER_BASE_NAME}.zip")
set(ZIP_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/portable")

# Configure the prepare script
configure_file(
  ${PROJECT_SOURCE_DIR}/CMakeModules/PreparePortableZip.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/PreparePortableZip.cmake
  @ONLY
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ZIP_OUTPUT_NAME}
  DEPENDS innosetup_install ${CMAKE_CURRENT_BINARY_DIR}/PreparePortableZip.cmake
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/PreparePortableZip.cmake
  COMMAND ${CMAKE_COMMAND} -E chdir ${ZIP_STAGING_DIR} ${CMAKE_COMMAND} -E tar cf ${CMAKE_CURRENT_BINARY_DIR}/${ZIP_OUTPUT_NAME} --format=zip .
  COMMENT "Building portable ZIP: ${ZIP_OUTPUT_NAME}"
  VERBATIM
)

add_custom_target(JellyfinDesktopZip
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${ZIP_OUTPUT_NAME}
)

add_custom_target(windows_zip DEPENDS JellyfinDesktopZip)

# Combined target for both installer and ZIP
add_custom_target(windows_all DEPENDS JellyfinDesktopInstaller JellyfinDesktopZip)

message(STATUS "Windows installer target configured: windows_package")
message(STATUS "Windows portable ZIP target configured: windows_zip")
message(STATUS "Windows combined target configured: windows_all")
