add_library(RTC INTERFACE)

target_include_directories(RTC INTERFACE ./include)

target_sources(RTC
        INTERFACE
        rtc.cpp
)

target_link_libraries(RTC
        INTERFACE
        pico_stdlib
)

