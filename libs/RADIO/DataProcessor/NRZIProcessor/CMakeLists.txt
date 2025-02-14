cmake_minimum_required(VERSION 3.13)
project(NRZIProcessor)

add_library(NRZIProcessor STATIC src/NRZIProcessor.cpp)

target_include_directories(NRZIProcessor PUBLIC ./include)

target_link_libraries(NRZIProcessor PUBLIC DataProcessor)
