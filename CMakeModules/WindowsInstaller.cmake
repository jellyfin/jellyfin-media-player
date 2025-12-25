# Find Inno Setup Compiler
find_program(ISCC iscc
  PATHS
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

message(STATUS "Windows installer target configured: windows_package")
