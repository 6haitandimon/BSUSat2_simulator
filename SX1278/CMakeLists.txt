add_library(SX1278 INTERFACE)

target_include_directories(SX1278 INTERFACE
       ./include
)

target_sources(SX1278
        INTERFACE
        sx1278.cpp
)

target_link_libraries(SX1278 INTERFACE
        pico_stdlib
        hardware_spi
        loraPacket
)
