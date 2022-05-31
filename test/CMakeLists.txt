
add_executable(config-test config-test.cpp)

target_link_libraries(config-test config)

target_include_directories(config-test PUBLIC
                        ${PROJECT_SOURCE_DIR}/include)


add_executable(signal-test signal-test.cpp)

target_link_libraries(signal-test mine_signal)

target_include_directories(signal-test PUBLIC
                        ${PROJECT_SOURCE_DIR}/include)