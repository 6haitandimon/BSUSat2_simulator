cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(ledPower C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_executable(main
  main.c
  ina219.c
)

target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Uncomment if you want to flash your board via picotool.
# pico_enable_stdio_usb(blink 1)
pico_add_extra_outputs(main)

target_link_libraries(main
  pico_stdlib
  hardware_i2c
  # Uncomment when using Pico W
  # pico_cyw43_arch_none
)


# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)


