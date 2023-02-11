aux_source_directory(src HTTP_SRC)

include_directories(src 3rdparty)

add_definitions(-DQHTTP_EXPORT)

set(CMAKE_AUTOMOC ON)
add_library(qhttp STATIC ${HTTP_SRC} 3rdparty/http-parser/http_parser.c)
target_link_libraries(qhttp
Qt6::Core
Qt6::Core5Compat
Qt6::Network
)
std_target_properties(qhttp)
