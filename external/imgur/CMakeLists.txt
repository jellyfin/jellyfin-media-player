# Find required packages
find_package(CURL REQUIRED)

include_directories(include)

# Define Imgur library
add_library(imgur STATIC
    imgur.cpp
    imgur.h
)

target_link_libraries(imgur PUBLIC
    CURL::libcurl
)