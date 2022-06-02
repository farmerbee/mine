
add_executable(config-test config-test.cpp)

target_link_libraries(config-test config)


add_executable(signal-test signal-test.cpp)

target_link_libraries(signal-test mine_signal)
