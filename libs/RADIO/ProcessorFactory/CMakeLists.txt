cmake_minimum_required(VERSION 3.13)
project(ProcessorFactory)

add_library(ProcessorFactory STATIC src/ProcessorFactory.cpp)

target_include_directories(ProcessorFactory PUBLIC ./include)

target_link_libraries(ProcessorFactory PUBLIC DataProcessor etl::etl)
