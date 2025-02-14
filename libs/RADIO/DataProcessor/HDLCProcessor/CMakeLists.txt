cmake_minimum_required(VERSION 3.13)
project(HDLCProcessor)

add_library(HDLCProcessor STATIC src/HDLCProcessor.cpp)

target_include_directories(HDLCProcessor PUBLIC ./include)

target_link_libraries(HDLCProcessor PUBLIC DataProcessor)
