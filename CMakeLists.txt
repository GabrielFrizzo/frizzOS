cmake_minimum_required(VERSION 3.15)
project(frizzOS C)

set(CMAKE_C_STANDARD 11)

include_directories(.)


add_executable(frizzOS
        datatypes.h
        pingpong.c
        pingpong.h
        queue.c
        queue.h
        pingpong-ipc.c
        pingpong-msg.c
        pingpong-mqueue.c
        )

target_link_libraries(frizzOS m) # enables math.h
