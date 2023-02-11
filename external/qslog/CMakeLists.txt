set(CMAKE_AUTOMOC ON)
aux_source_directory(. QSLOG_SRC)
add_library(qslog STATIC ${QSLOG_SRC})
std_target_properties(qslog)
target_link_libraries(qslog
Qt6::Core
Qt6::Core5Compat
)
