cmake_minimum_required(VERSION 3.9)
project(libevent_echo C)

set(CMAKE_C_STANDARD 99)

add_executable(libevent_echo main.c)
target_link_libraries(libevent_echo event)