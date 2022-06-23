
add_executable(config-test config-test.cpp)

target_link_libraries(config-test config)


add_executable(signal-test signal-test.cpp)

target_link_libraries(signal-test mine_signal)

add_executable(log-test log-test.cpp)

target_link_libraries(log-test log) 

add_executable(thread-test thread-test.cpp)

target_link_libraries(thread-test thread) 
