cmake_minimum_required(VERSION 3.13)
project(RADIO)

add_subdirectory(DataProcessor)
add_subdirectory(ProcessorFactory)
add_library(RADIO INTERFACE)

target_link_libraries(RADIO INTERFACE ProcessorFactory DataProcessor etl::etl)
