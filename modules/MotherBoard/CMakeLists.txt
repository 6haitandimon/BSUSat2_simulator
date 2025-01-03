file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.h)

add_executable(MotherBoard
        ${SOURCES}
)

target_link_libraries(MotherBoard
        PRIVATE
        pico_stdlib
        hardware_adc
        hardware_flash
        hardware_sync
        hardware_clocks
        hardware_rtc
        PCA9554
        MCP2515
        INA3221
        INA219
        INTERRUPT
        CANTRANSMIT
        RTC
)

pico_enable_stdio_usb(MotherBoard 0)
pico_enable_stdio_uart(MotherBoard 0)

pico_add_extra_outputs(MotherBoard)

set_property(TARGET MotherBoard APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")
