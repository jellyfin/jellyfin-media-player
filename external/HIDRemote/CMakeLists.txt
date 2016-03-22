set_source_files_properties(HIDRemote.m PROPERTIES COMPILE_FLAGS -fno-objc-arc)
add_library(hidremote STATIC HIDRemote.m HIDRemote.h)
