cmake_minimum_required(VERSION 3.16)
project(MixeryPiC C)

set(CMAKE_C_STANDARD 99)
INCLUDE_DIRECTORIES(lib)
link_libraries(wiringPi wiringPiDev pthread)

add_executable(MixeryPi main.c src/LCD.c src/network.c)