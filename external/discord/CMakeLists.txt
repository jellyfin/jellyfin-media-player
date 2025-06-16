set(DISCORD_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(DISCORD_SDK_LIB_DIR "${DISCORD_SDK_ROOT}/lib")
set(DISCORD_SDK_BIN_DIR "${DISCORD_SDK_ROOT}/bin")
set(DISCORD_SDK_INCLUDE_DIR "${DISCORD_SDK_ROOT}/include")

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/x64/discord-rpc.lib")
        set(DISCORD_SHARED_LIB "${DISCORD_SDK_BIN_DIR}/x64/discord-rpc.dll")
    else()
        set(DISCORD_LIB_PATH "${DISCORD_SDK_LIB_DIR}/x86/discord-rpc.lib")
        set(DISCORD_SHARED_LIB "${DISCORD_SDK_BIN_DIR}/x86/discord-rpc.dll")
    endif()
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

target_link_libraries(discord ${DISCORD_LIB_PATH})

if (WIN32)
    install(FILES "${DISCORD_SHARED_LIB}" DESTINATION .)
elseif(UNIX)
    install(FILES "${DISCORD_SHARED_LIB}" DESTINATION /lib)
endif()

