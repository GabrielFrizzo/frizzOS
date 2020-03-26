cmake_minimum_required(VERSION 3.15)
project(p02 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(p02
        datatypes.h
        pingpong-tasks3.c
        pingpong.h pingpong.c)
