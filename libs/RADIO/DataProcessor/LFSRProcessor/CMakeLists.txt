cmake_minimum_required(VERSION 3.13)
project(LFSRProcessor)

add_library(LFSRProcessor STATIC src/LFSRProcessor.cpp)

target_include_directories(LFSRProcessor PUBLIC ./include)

target_link_libraries(LFSRProcessor PUBLIC DataProcessor)
