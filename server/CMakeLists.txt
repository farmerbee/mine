add_library(mine_signal mine_signal.cpp)

add_executable(server server.cpp)

target_link_libraries(server config daemonize process)