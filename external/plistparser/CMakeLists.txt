include_directories(.)

add_library(plistparser STATIC
  plistparser.cpp
  plistparser.h
  plistserializer.cpp
  plistserializer.h
)
std_target_properties(plistparser)
target_link_libraries(plistparser
Qt6::Core
Qt6::Core5Compat
Qt6::Xml
)
