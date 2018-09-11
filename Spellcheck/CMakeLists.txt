cmake_minimum_required(VERSION 3.9)
project(Spellcheck C)

set(CMAKE_C_STANDARD 99)

add_executable(server server.c queue.c queue.h)
add_executable(client client.c)