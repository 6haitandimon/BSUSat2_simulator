#include "rtc.h"

namespace RTC{

    RTC::RTC(uint8_t _clk, uint8_t _dio, uint8_t _cs) : clk(_clk), dio(_dio), cs(_cs){
        gpio_init(clk);
        gpio_init(dio);
        gpio_init(cs);

        gpio_set_dir(clk, GPIO_OUT);
        gpio_set_dir(cs, GPIO_OUT);

    }

    uint16_t RTC::dec2hex(uint16_t data) {
        return ((data / 10) * 16 + (data % 10));
    }

    uint16_t RTC::hex2dec(uint16_t data) {
        return ((data / 16) * 10 + (data % 16));
    }

    void RTC::writeByte(uint8_t data) {
        gpio_set_dir(this->dio, GPIO_OUT);
        for(uint8_t i = 0; i < 8; i++){
            gpio_put(this->dio, (data >> i) & 1);
            gpio_put(this->clk, 1);
            busy_wait_us(20);
            gpio_put(this->clk, 0);
            busy_wait_us(20);
        }

    }

    uint8_t RTC::readByte() {
        uint8_t data = 0;
        gpio_set_dir(this->dio, GPIO_IN);
        for(uint8_t i = 0; i < 8; i++){
            data = data | (gpio_get(this->dio) << i);
            gpio_put(this->clk, 1);
            busy_wait_us(20);
            gpio_put(this->clk, 0);
            busy_wait_us(20);
        }
        return data;
    }

    uint8_t RTC::getReg(uint8_t reg) {
        gpio_put(this->cs, 1);
        writeByte(reg);
        uint8_t data = readByte();

        gpio_put(this->cs, 0);

        return data;
    }

    void RTC::setReg(uint8_t reg, uint8_t data) {
        gpio_put(this->cs, 1);
        writeByte(reg);
        writeByte(data);
        gpio_put(this->cs, 0);
    }

    void RTC::wr(uint8_t reg, uint8_t data) {
        setReg(DS1302_REG_WP, 0);
        setReg(reg, data);
        setReg(DS1302_REG_WP, 0x80);

    }

    void RTC::start() {
        uint8_t data = getReg(DS1302_REG_SECOND + 1);
        wr(DS1302_REG_SECOND, data & 0x7f);
    }

    void RTC::stop() {
        uint8_t data = getReg(DS1302_REG_SECOND + 1);
        wr(DS1302_REG_SECOND, data | 0x80);
    }

    uint8_t RTC::second(uint8_t seconds) {
        if(seconds == 0)
            return (hex2dec(getReg(DS1302_REG_SECOND + 1)) % 60);
        else
            wr(DS1302_REG_SECOND, dec2hex(seconds % 60));
        return 0;
    }

    uint8_t RTC::minute(uint8_t minutes) {
        if(minutes == 0)
            return (hex2dec(getReg(DS1302_REG_MINUTE + 1)));
        else
            wr(DS1302_REG_MINUTE, dec2hex(minutes % 60));
        return 0;
    }

    uint8_t RTC::hour(uint8_t hours) {
        if(hours == 0)
            return (hex2dec(getReg(DS1302_REG_HOUR + 1)));
        else
            wr(DS1302_REG_HOUR, dec2hex(hours % 24));
        return 0;
    }

    uint8_t RTC::weekday(uint8_t weekdays) {
        if(weekdays == 0)
            return (hex2dec(getReg(DS1302_REG_WEEKDAY + 1)));
        else
            wr(DS1302_REG_WEEKDAY, dec2hex(weekdays % 8));
        return 0;
    }

    uint8_t RTC::day(uint8_t days) {
        if(days == 0)
            return (hex2dec(getReg(DS1302_REG_DAY + 1)));
        else
            wr(DS1302_REG_DAY, dec2hex(days % 32));
        return 0;
    }

    uint8_t RTC::mouth(uint8_t mouths) {
        if(mouths == 0)
            return (hex2dec(getReg(DS1302_REG_MONTH + 1)));
        else
            wr(DS1302_REG_MONTH, dec2hex(mouths % 13));
        return 0;
    }

    uint16_t RTC::year(uint16_t years) {
        if(years == 0)
            return (hex2dec(getReg(DS1302_REG_YEAR + 1)) + 2000);
        else
            wr(DS1302_REG_YEAR, dec2hex(years % 100));
        return 0;
    }

    uint8_t RTC::ram(uint8_t reg, uint8_t data) {
        if(data == 0)
            return(getReg(DS1302_REG_RAM + 1 + (reg % 31) * 2));
        else
            wr(DS1302_REG_RAM + (reg % 31) * 2, data);
        return 0;
    }

    void RTC::dataTime(uint16_t *data, bool flag) {
        if(flag == 0){
            data[0] = year();
            data[1] = mouth();
            data[2] = day();
            data[3] = weekday();
            data[4] = hour();
            data[5] = minute();
            data[6] = second();
        }
        else{
            year(data[0]);
            mouth(static_cast<uint8_t>(data[1]));
            day(static_cast<uint8_t>(data[2]));
            weekday(static_cast<uint8_t>(data[3]));
            hour(static_cast<uint8_t>(data[4]));
            minute(static_cast<uint8_t>(data[5]));
            second(static_cast<uint8_t>(data[6]));
        }
    }

    uint32_t RTC::dataTimeUnix(){
        std::tm timeInfo = {};
        timeInfo.tm_year = year() - 1900;
        busy_wait_us(40);
        timeInfo.tm_mon = mouth() - 1;
        busy_wait_us(40);
        timeInfo.tm_mday = day();
        busy_wait_us(40);
        timeInfo.tm_hour = hour();
        busy_wait_us(40);
        timeInfo.tm_min = minute();
        busy_wait_us(40);
        timeInfo.tm_sec = second();
        busy_wait_us(40);

        std::time_t unixTime = std::mktime(&timeInfo);

        if(unixTime == -1){
            return 0;
        }
        else{
            return static_cast<uint32_t>(unixTime);
        }
    }
}













