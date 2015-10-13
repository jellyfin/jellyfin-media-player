
# SPMediaKeyTap uses some deprecated methods, no need to fix that right now, so let's supress the warnings.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")

add_library(spmediakeytap STATIC
  SPMediaKeyTap.h SPMediaKeyTap.m
  SPInvocationGrabbing/NSObject+SPInvocationGrabbing.m
  SPInvocationGrabbing/NSObject+SPInvocationGrabbing.h
)