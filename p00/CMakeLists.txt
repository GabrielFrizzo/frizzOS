cmake_minimum_required(VERSION 3.15)
project(p00 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(p00
        queue.h
        queue.c
        testafila.c)
