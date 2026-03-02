cmake_minimum_required(VERSION 4.0)
set(CMAKE_CXX_STANDARD 20)

project(fly)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(EMSCRIPTEN "Build web" Off)

add_subdirectory(src)
