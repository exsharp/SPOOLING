cmake_minimum_required(VERSION 3.3)
project(SPOOLING)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.c request.h request.c)
add_executable(SPOOLING ${SOURCE_FILES})