
add_library(config config.cpp)
add_library(strproc strproc.cpp)

target_link_libraries(config strproc)
