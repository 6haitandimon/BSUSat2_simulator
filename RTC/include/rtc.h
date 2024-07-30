#ifndef MATHERBOARD_RTC_H
#define MATHERBOARD_RTC_H
#include "pico/stdlib.h"
#include <ctime>
namespace RTC{

    enum RTC_REGISTR{
        DS1302_REG_SECOND = 0x80,
        DS1302_REG_MINUTE = 0x82,
        DS1302_REG_HOUR   = 0x84,
        DS1302_REG_DAY    = 0x86,
        DS1302_REG_MONTH  = 0x88,
        DS1302_REG_WEEKDAY= 0x8A,
        DS1302_REG_YEAR   = 0x8C,
        DS1302_REG_WP     = 0x8E,
        DS1302_REG_CTRL   = 0x90,
        DS1302_REG_RAM    = 0xC0
    };

    class RTC{
    private:

        uint32_t date;
        uint32_t time;
        uint8_t clk;
        uint8_t dio;
        uint8_t cs;

        uint16_t dec2hex(uint16_t data);
        uint16_t hex2dec(uint16_t data);

        void writeByte(uint8_t data);
        uint8_t readByte();

        uint8_t getReg(uint8_t reg);
        void setReg(uint8_t reg, uint8_t data);

        void wr(uint8_t reg, uint8_t data);

    public:
//    rtc(){}
        RTC(uint8_t _clk, uint8_t _dio, uint8_t _cs);
        void start();
        void stop();

        uint8_t second(uint8_t seconds = 0);
        uint8_t minute(uint8_t minutes = 0);
        uint8_t hour(uint8_t hours = 0);

        uint8_t weekday(uint8_t weekdays = 0);
        uint8_t day(uint8_t days = 0);
        uint8_t mouth(uint8_t mouths = 0);
        uint16_t year(uint16_t years = 0);

        void dataTime(uint16_t *data, bool flag = false);
        uint8_t ram(uint8_t reg, uint8_t data = 0);

        uint32_t dataTimeUnix();
    };
}
#endif
