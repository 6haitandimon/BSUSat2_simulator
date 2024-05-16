cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(radioDriver C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(radioDriver
        main.cpp
)

add_subdirectory(e10_433)
add_subdirectory(etl)

target_link_libraries(radioDriver
        pico_stdlib
        hardware_spi
        e10_433
        etl::etl
)


pico_enable_stdio_usb(radioDriver 0)
pico_enable_stdio_uart(radioDriver 1)

pico_add_extra_outputs(radioDriver)
