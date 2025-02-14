cmake_minimum_required(VERSION 3.13)
project(AX25Processor)

add_library(AX25Processor STATIC src/AX25Processor.cpp)

target_include_directories(AX25Processor PUBLIC include)

target_link_libraries(AX25Processor PUBLIC DataProcessor)