include_directories(.)

file(GLOB DISCORD_SRC
     "*.h"
     "*.cpp"
)

add_library(discord STATIC
  ${DISCORD_SRC}
)
# add_library(discord discord_game_sdk.dll.lib)
target_link_libraries(discord D:/Projects/jellyfin-media-player/external/discord/discord_game_sdk.dll.lib)
set_target_properties(discord PROPERTIES LINKER_LANGUAGE C)
