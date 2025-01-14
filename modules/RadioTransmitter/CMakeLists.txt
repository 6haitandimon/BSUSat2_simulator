file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS include/*.h)

add_executable(RadioTransmitter
        ${HEADERS}
        ${SOURCES}
)

target_link_libraries(RadioTransmitter
        PRIVATE
        pico_stdlib
        SX127x
        etl::etl
)

pico_enable_stdio_usb(RadioTransmitter 0)
pico_enable_stdio_uart(RadioTransmitter 0)

pico_add_extra_outputs(RadioTransmitter)

set_property(TARGET RadioTransmitter APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")
