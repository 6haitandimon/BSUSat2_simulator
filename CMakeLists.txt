cmake_minimum_required(VERSION 3.13)

set(CMAKE C_STANDARD 99)
set(CMAKE CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(mcp2515 C CXX ASM)

pico_sdk_init()

add_executable(mcp2515

    #Library executable
    MCP2515/include/mcp2515.c

    #main 
    src/main.c

)

target_include_directories(mcp2515 PUBLIC MCP2515/)

pico_set_program_name(mcp2515 "mcp2515_driver")
pico_set_program_version(mcp2515 "0.1")

pico_enable_stdio_uart(mcp2515 0)
pico_enable_stdio_usb(mcp2515 1)

target_link_libraries(mcp2515
    pico_stdlib
    hardware_spi
)

pico_add_extra_outputs(mcp2515)





