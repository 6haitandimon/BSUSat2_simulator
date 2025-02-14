cmake_minimum_required(VERSION 3.13)
project(DataProcessor)

add_library(DataProcessor INTERFACE)

target_include_directories(DataProcessor INTERFACE ./include)

add_subdirectory(AX25Processor)
add_subdirectory(HDLCProcessor)
add_subdirectory(LFSRProcessor)
add_subdirectory(NRZIProcessor)
add_subdirectory(Tools)

target_link_libraries(DataProcessor INTERFACE AX25Processor HDLCProcessor LFSRProcessor NRZIProcessor Tools etl::etl)
