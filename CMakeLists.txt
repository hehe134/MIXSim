cmake_minimum_required(VERSION 3.10)
project(MIXSim C)

set(CMAKE_C_STANDARD 99)

add_executable(MIXSim Simulator.c Word.c Word.h)