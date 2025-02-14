cmake_minimum_required(VERSION 3.13)
project(TimerManager)

file(GLOB SOURCES src/*.cpp)

add_library(TimerManager STATIC ${SOURCES})

target_include_directories(TimerManager PUBLIC ./include)


target_link_libraries(TimerManager
        PRIVATE
        pico_stdlib
        hardware_timer
        etl::etl
)
