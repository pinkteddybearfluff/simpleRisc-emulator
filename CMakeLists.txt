cmake_minimum_required(VERSION 4.1)
project(simpRisc C)

set(CMAKE_C_STANDARD 11)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
#add_link_options(-fsanitize=address)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion")

add_executable(simpRisc
        src/main.c
        src/utils.c
        src/utils.h
        src/cpu.c
        src/cpu.h
        src/memory.c
        src/memory.h
        src/isa.c
        src/isa.h
        src/decode.c
        src/decode.h
        src/execute.c
        src/execute.h
        src/assembler.c
        src/assembler.h)