
add_library(config config.cpp)
add_library(strproc strproc.cpp)

target_link_libraries(config strproc)
target_include_directories(config PUBLIC
                        ${PROJECT_SOURCE_DIR}/include)

target_include_directories(strproc PUBLIC
                        ${PROJECT_SOURCE_DIR}/include)

