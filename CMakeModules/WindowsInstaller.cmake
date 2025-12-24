# Qt Installer Framework configuration for Windows

find_program(QTIFW_BINARYCREATOR binarycreator
    HINTS
        ENV QTIFW_DIR
        ENV IQTA_TOOLS
        "$ENV{IQTA_TOOLS}/QtInstallerFramework"
    PATH_SUFFIXES bin 4.8/bin 4.7/bin 4.6/bin
)

if(NOT QTIFW_BINARYCREATOR)
    message(WARNING "Qt Installer Framework not found. windows_package target will not be available.")
    return()
endif()

get_filename_component(QTIFW_DIR ${QTIFW_BINARYCREATOR} DIRECTORY)
message(STATUS "Found Qt Installer Framework: ${QTIFW_DIR}")

set(QTIFW_CONFIG_DIR "${PROJECT_SOURCE_DIR}/bundle/win/qtifw/config")
set(QTIFW_PACKAGES_DIR "${PROJECT_SOURCE_DIR}/bundle/win/qtifw/packages")
set(QTIFW_PACKAGE_NAME "org.jellyfin.JellyfinDesktop")
set(QTIFW_PACKAGE_DATA_DIR "${QTIFW_PACKAGES_DIR}/${QTIFW_PACKAGE_NAME}/data")

# Configure version in config.xml and package.xml
string(TIMESTAMP RELEASE_DATE "%Y-%m-%d")
# Define Qt IFW variables so configure_file passes them through unchanged
set(ApplicationsDir "@ApplicationsDir@")
set(TargetDir "@TargetDir@")
configure_file(
    "${QTIFW_CONFIG_DIR}/config.xml"
    "${CMAKE_BINARY_DIR}/qtifw/config/config.xml"
    @ONLY
)
configure_file(
    "${QTIFW_PACKAGES_DIR}/${QTIFW_PACKAGE_NAME}/meta/package.xml"
    "${CMAKE_BINARY_DIR}/qtifw/packages/${QTIFW_PACKAGE_NAME}/meta/package.xml"
    @ONLY
)

# Copy static config files
file(COPY "${QTIFW_CONFIG_DIR}/controlscript.qs" DESTINATION "${CMAKE_BINARY_DIR}/qtifw/config/")
file(COPY "${QTIFW_CONFIG_DIR}/jellyfin.ico" DESTINATION "${CMAKE_BINARY_DIR}/qtifw/config/")
file(GLOB QTIFW_META_FILES
    "${QTIFW_PACKAGES_DIR}/${QTIFW_PACKAGE_NAME}/meta/*.qs"
    "${QTIFW_PACKAGES_DIR}/${QTIFW_PACKAGE_NAME}/meta/*.ui"
)
file(COPY ${QTIFW_META_FILES}
     DESTINATION "${CMAKE_BINARY_DIR}/qtifw/packages/${QTIFW_PACKAGE_NAME}/meta/")

# Determine architecture string
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(INSTALLER_ARCH_STR "windows-arm64")
    else()
        set(INSTALLER_ARCH_STR "windows-x64")
    endif()
else()
    set(INSTALLER_ARCH_STR "windows-x86")
endif()

set(INSTALLER_OUTPUT "JellyfinDesktop-${VERSION_STRING}-${INSTALLER_ARCH_STR}.exe")

# Target to copy installed files to package data directory
add_custom_target(qtifw_prepare_data
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/qtifw/packages/${QTIFW_PACKAGE_NAME}/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_INSTALL_PREFIX}" "${CMAKE_BINARY_DIR}/qtifw/packages/${QTIFW_PACKAGE_NAME}/data"
    DEPENDS JellyfinDesktop
    COMMENT "Preparing QtIFW package data..."
)

# Main installer target
add_custom_target(windows_package
    COMMAND ${CMAKE_COMMAND} -P cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/qtifw/packages/${QTIFW_PACKAGE_NAME}/data"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_INSTALL_PREFIX}" "${CMAKE_BINARY_DIR}/qtifw/packages/${QTIFW_PACKAGE_NAME}/data"
    COMMAND ${QTIFW_BINARYCREATOR}
        --offline-only
        -c "${CMAKE_BINARY_DIR}/qtifw/config/config.xml"
        -p "${CMAKE_BINARY_DIR}/qtifw/packages"
        "${CMAKE_BINARY_DIR}/${INSTALLER_OUTPUT}"
        "${QTIFW_PACKAGE_NAME}"
    DEPENDS JellyfinDesktop
    COMMENT "Creating Windows installer with Qt Installer Framework..."
)
