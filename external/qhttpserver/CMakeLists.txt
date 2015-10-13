aux_source_directory(src HTTP_SRC)

include_directories(http-parser)
add_library(http-parser STATIC http-parser/http_parser.c)

set(CMAKE_AUTOMOC ON)
add_library(qhttpserver STATIC ${HTTP_SRC} ${PARSER_SRC})
target_link_libraries(qhttpserver http-parser)