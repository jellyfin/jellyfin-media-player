set(APPLE_SRC
    HIDRemote/HIDRemote.h HIDRemote/HIDRemote.m
    InputAppleRemote.h InputAppleRemote.mm
    AppleRemoteDelegate.mm AppleRemoteDelegate.h
    ${HID_SRC} InputAppleMediaKeys.h InputAppleMediaKeys.mm
)
add_sources(${APPLE_SRC})