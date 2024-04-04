add_library(CANTRANSMIT INTERFACE)

target_include_directories(CANTRANSMIT INTERFACE ./include)

target_sources(CANTRANSMIT
        INTERFACE
        can.cpp
)

target_link_libraries(CANTRANSMIT
        INTERFACE
        pico_stdlib
        MCP2515
)