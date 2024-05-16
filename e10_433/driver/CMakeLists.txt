add_library(driver INTERFACE)

target_include_directories(driver INTERFACE ./include)

target_sources(driver
        INTERFACE
        driver.cpp
)

target_link_libraries(driver
        INTERFACE
        pico_stdlib
        hardware_spi
        etl::etl
)