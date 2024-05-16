add_library(AX25 INTERFACE)

target_include_directories(AX25 INTERFACE ./include)

target_sources(AX25
        INTERFACE
        AX25.cpp
)

target_link_libraries(AX25
        INTERFACE
        pico_stdlib
)