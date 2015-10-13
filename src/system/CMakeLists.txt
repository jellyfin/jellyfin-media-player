if (OPENELEC)
add_subdirectory(openelec)
endif(OPENELEC)

find_all_sources(. SRC)
add_sources(${SRC})