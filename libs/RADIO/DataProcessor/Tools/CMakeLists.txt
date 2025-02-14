cmake_minimum_required(VERSION 3.13)
project(Tools)

add_library(Tools STATIC src/Tools.cpp)

target_include_directories(Tools PUBLIC ./include)

target_link_libraries(Tools PUBLIC DataProcessor)