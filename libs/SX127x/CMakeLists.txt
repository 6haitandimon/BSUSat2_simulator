add_library(SX127x INTERFACE)

target_include_directories(SX127x INTERFACE ./include)

target_sources(SX127x
        INTERFACE
        sx127x.cpp
)

target_link_libraries(SX127x
        INTERFACE
        pico_stdlib
        hardware_spi
        etl::etl
)

