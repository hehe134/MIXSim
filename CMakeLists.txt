cmake_minimum_required(VERSION 3.10)
project(MIXSim C)

set(CMAKE_C_STANDARD 99)

add_executable(MIXSim cmake-build-debug/Simulator.c cmake-build-debug/Word.c cmake-build-debug/Word.h)