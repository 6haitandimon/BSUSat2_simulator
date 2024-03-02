cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(BSUSat2_simulator C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(BSUSat2_simulator
  main.c
)


target_include_directories(BSUSat2_simulator PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

# Uncomment if you want to flash your board via picotool.
# pico_enable_stdio_usb(blink 1)
pico_add_extra_outputs(BSUSat2_simulator)
add_subdirectory(INA3221)
add_subdirectory(INA219)
target_link_libraries(BSUSat2_simulator INA3221 INA219)


target_link_libraries(BSUSat2_simulator
  pico_stdlib
  hardware_i2c
  # Uncomment when using Pico W
  # pico_cyw43_arch_none
)


# enable usb output, disable uart output
pico_enable_stdio_usb(BSUSat2_simulator 1)
pico_enable_stdio_uart(BSUSat2_simulator 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(BSUSat2_simulator)
