add_library(e10_433 INTERFACE)

target_include_directories(e10_433 INTERFACE ./include)


target_sources(e10_433
        INTERFACE
        e10_433.cpp
)

add_subdirectory(AX25)
add_subdirectory(driver)


target_link_libraries(e10_433
        INTERFACE
        pico_stdlib
        hardware_spi
        AX25
        driver
)