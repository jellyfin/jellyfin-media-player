include_directories(.)

add_library(plistparser STATIC
  plistparser.cpp
  plistparser.h
  plistserializer.cpp
  plistserializer.h
)
std_target_properties(plistparser)
