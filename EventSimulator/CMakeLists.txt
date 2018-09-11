cmake_minimum_required(VERSION 3.9)
project(EventSimulator C)

set(CMAKE_C_STANDARD 99)

add_executable(EventSimulator main.c globals.c globals.h cpu.c cpu.h disk.c disk.h logging.c logging.h)