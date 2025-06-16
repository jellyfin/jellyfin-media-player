# Define some handy Social SDK variables
set(DISCORD_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(DISCORD_SDK_LIB_DIR "${DISCORD_SDK_ROOT}/lib")
set(DISCORD_SDK_BIN_DIR "${DISCORD_SDK_ROOT}/bin/release")
set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_ROOT}/include")

# Platform-specific Social SDK library paths
if(WIN32)
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/discord_partner_sdk.lib")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_BIN_DIR}/discord_partner_sdk.dll")
elseif(APPLE)
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord-rpc.dylib")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord-rpc.dylib")
    
else() # Linux
    set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/libdiscord-rpc.so")
    set(DISCORD_SHARED_LIB "${DISCORD_SDK_LIB_DIR}/libdiscord-rpc.so")
endif()

include_directories(include)

add_library(discord STATIC
    include/discord_register.h
    include/discord_rpc.h
    discord_impl.cpp
)

# Link the Social SDK library
target_link_libraries(discord ${DISCORD_LIB_PATH})

if (WIN32)
    install(FILES "${DISCORD_SHARED_LIB}" DESTINATION .)
elseif(UNIX)
    install(FILES "${DISCORD_SHARED_LIB}" DESTINATION /lib)
endif()

