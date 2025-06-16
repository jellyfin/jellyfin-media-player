set(CURL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(CURL_LIBRARY_STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/libcurl_a.lib")
set(CURL_LIBRARY_SHARED "${CMAKE_CURRENT_SOURCE_DIR}/bin/libcurl.dll")

if(WIN32)
    add_definitions(-DCURL_STATICLIB)
endif()

add_library(imgur STATIC
    imgur.cpp
    imgur.h
)

target_include_directories(imgur PUBLIC
    ${CURL_INCLUDE_DIR}
)

target_link_libraries(imgur PUBLIC
    ${CURL_LIBRARY_STATIC}
    ws2_32
    wldap32
    crypt32
    normaliz
)

if(WIN32)
    install(FILES "${CURL_LIBRARY_SHARED}" DESTINATION .)
endif()