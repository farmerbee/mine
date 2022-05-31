
add_executable(config-test config-test.cpp)

target_link_libraries(config-test config)

target_include_directories(config-test PUBLIC
                        ${PROJECT_SOURCE_DIR}/include)