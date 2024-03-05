cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(Slot_control C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# include_directories(PCA9554/include)
file(GLOB SRC_FILE ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB PCA9554_SRC_FILE ${PROJECT_SOURCE_DIR}/PCA9554/*.cpp)

add_executable(Slot_control
        ${SRC_FILE}
        # src/main.cpp
        ${PCA9554_SRC_FILE}
        # PCA9554/PCA9554.cpp
)

target_include_directories(Slot_control PUBLIC PCA9554/include)

if(SRC_FILE)
    message("Переменная SRC_FILE не пустая и содержит файлы")
else()
    message(FATAL_ERROR "Переменная SRC_FILE пустая или не содержит файлов")
endif()

# target_include_directories(Slot_control PUBLIC PCA9554/)
# pico_enable_stdio_usb(Slot_control 1)
# pico_add_extra_outputs(Slot_control)


target_link_libraries(Slot_control
  pico_stdlib
  hardware_i2c
  hardware_adc
)


pico_enable_stdio_usb(Slot_control 1)
pico_enable_stdio_uart(Slot_control 0)

pico_add_extra_outputs(Slot_control)
