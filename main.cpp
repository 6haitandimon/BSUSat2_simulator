#include "PCA9554.h"
#include "mcp2515.h"
#include "ina3221.h"
#include "INA219.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"
#include "hardware/rtc.h"
#include "interrupt.h"
#include <ctime>
#include "rtc.h"

#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - 2 * sizeof(uint16_t) - sizeof(uint32_t))

const uint16_t *flash_target_counter = (const uint16_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
const uint32_t *flash_target_time = (const uint32_t *)(XIP_BASE + FLASH_TARGET_OFFSET + 2 * sizeof(uint16_t));

uint16_t read_counter_from_flash() {
    return *flash_target_counter;
}

void write_counter_to_flash(uint16_t counter) {
    uint32_t ints = save_and_disable_interrupts();
    uint8_t data[FLASH_PAGE_SIZE] = {0};
    ((uint16_t *)data)[0] = counter;
    flash_range_erase(FLASH_TARGET_OFFSET, 2 * sizeof(uint16_t) + sizeof(uint32_t));
    flash_range_program(FLASH_TARGET_OFFSET, data, sizeof(uint16_t));
    restore_interrupts(ints);
}

uint32_t read_time_from_flash() {
    return *flash_target_time;
}

void write_time_to_flash(uint32_t time) {
    uint32_t ints = save_and_disable_interrupts();
    uint8_t data[FLASH_PAGE_SIZE] = {0};
    ((uint32_t *)data)[0] = time;
    flash_range_program(FLASH_TARGET_OFFSET + 2 * sizeof(uint16_t), data, sizeof(uint32_t));
    restore_interrupts(ints);
}



MCP2515 can0 = MCP2515(spi0, 17, 19, 16, 18);

INA3221::INA3221 ina3221_1 = INA3221::INA3221(i2c0, 20, 21, 0x42, 4.2, 3.3, 0.2);
INA3221::INA3221 ina3221_2 = INA3221::INA3221(i2c0, 20, 21, 0x43, 4.2, 3.3, 0.2);

INA3221::INA3221 ina3221_solar_1 = INA3221::INA3221(i2c0, 20, 21, 0x40, 4.2, 3.3, 0.2);
INA3221::INA3221 ina3221_solar_2 = INA3221::INA3221(i2c0, 20, 21, 0x41, 4.2, 3.3, 0.2);

PCA9554 pca9554_1 = PCA9554(i2c0, 21, 20, 0x3F, 0x40);
PCA9554 pca9554_2 = PCA9554(i2c0, 21, 20, 0x38, 0x70);

INA219::INA219 ina219_slot = INA219::INA219(i2c0, 20, 21,  0x44, 0.150, 4.2, 3.2, 0.1, 4096);

RTC::RTC rtc = RTC::RTC(10, 11, 12);

float convertionFactor = 3.009 / (4095.0) * (5.0 / 3.0);
double A = 0.8781625571e-3;

double B = 2.531972392e-4;
double C = 1.840753501e-7;


uint16_t MatherBoardTelemtry[62];


void ADCInit() {
    adc_init();
    adc_gpio_init(28); //bat
    adc_gpio_init(27); //usb
    adc_gpio_init(26); //solar
    adc_set_temp_sensor_enabled(true);
    return;
}

uint16_t adc_config_and_read_u16(uint8_t channel) {
    adc_select_input(channel);
    uint32_t raw = adc_read();
    const uint32_t bits = 12;

    return raw << (16 - bits) | raw >> (2 * bits - 16);
}

uint16_t cpu_temperature_read_raw(){
    adc_select_input(4);
    uint16_t temp = adc_read();

    return temp;
}

uint16_t adc_read_u16_raw(uint8_t channel) {
    adc_select_input(channel);
    uint16_t raw = adc_read();
    return raw;
}

float ADCRead(uint8_t channel){
    float value = (adc_config_and_read_u16(channel) >> 4) * convertionFactor;
    return value;
}

uint16_t ADCReadRAW(uint8_t channel) {
    return adc_read_u16_raw(channel);
}

void slotInit(){
    pca9554_1.slotAdd(1, 0x02);
    pca9554_1.slotAdd(2, 0x04);
    pca9554_1.slotAdd(8, 0x20);
    pca9554_1.slotAdd(9, 0x80);
    pca9554_1.slotAdd(10, 0x10);

    pca9554_2.slotAdd(3, 0x01);
    pca9554_2.slotAdd(4, 0x02);
    pca9554_2.slotAdd(5, 0x04);
    pca9554_2.slotAdd(6, 0x08);
    pca9554_2.slotAdd(7, 0x80);

    return;
}

void getActiveParametrs(uint8_t slotNumber, uint8_t dataNumber){
    ina219_slot.changeAddres(0x44 + slotNumber - 1);
    MatherBoardTelemtry[dataNumber++] = ina219_slot.get_voltageRAW();
    MatherBoardTelemtry[dataNumber] = ina219_slot.get_current_from_shunt_in_mARAW();

    return;
}

uint16_t getEnambleStatus(){
    uint16_t data = 0;
    data = ((1 & pca9554_1.getEnableFlag(10)) << 9) | \
    ((1 & pca9554_1.getEnableFlag(9)) << 8)  | \
    ((1 & pca9554_1.getEnableFlag(8)) << 7) |  \
    ((1 & pca9554_2.getEnableFlag(7)) << 6) | \
    ((1 & pca9554_2.getEnableFlag(6)) << 5) | \
    ((1 & pca9554_2.getEnableFlag(5)) << 4) | \
    ((1 & pca9554_2.getEnableFlag(4)) << 3) | \
    ((1 & pca9554_2.getEnableFlag(3)) << 2) | \
    ((1 & pca9554_1.getEnableFlag(2)) << 1) | \
    ((1 & pca9554_1.getEnableFlag(1)));
    return data;
}

uint32_t last_reset_time;
uint16_t ResetCounter;
//void timeSetup(uint16_t* dataTime){
////    rtc.stop();
//    rtc.dataTime(dataTime, true);
//    rtc.start();
//}

//bool reserved_addr(uint8_t addr) {
//    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
//}

int main() {
    stdio_init_all();
//    uint16_t dataTime[7] = {2024, 5, 28, 2, 10, 17, 30};
//    timeSetup(dataTime);
    last_reset_time = read_time_from_flash();
    ResetCounter = read_counter_from_flash();

    uint32_t current_time = rtc.dataTimeUnix();

    ResetCounter++;

    write_counter_to_flash(ResetCounter);
    write_time_to_flash(current_time);

    ADCInit();
    slotInit();
    can0.reset();
    can0.setBitrate(CAN_1000KBPS, MCP_16MHZ);
    INTERRUPT::MCPInterruptSetup();
    can0.setNormalMode();

    pca9554_1.enableSlot(2);
    pca9554_2.enableSlot(7);
    pca9554_1.enableSlot(9);

    uint8_t sec;

    while (true) {
//        uint16_t datatime[7];
//        rtc.dataTime(dataTime);
//         sec = rtc.second();
//        printf("%d", dataTime[6]);
//        printf("hours: %d ", rtc.hour());
//        printf("minutes: %d ", rtc.minute());
//        printf("seconds: %d\n\n", rtc.second());
        MatherBoardTelemtry[0] = ina3221_1.GetVoltageRAW(1);
        MatherBoardTelemtry[1] = ina3221_1.GetCurrentRAW(1);

        MatherBoardTelemtry[2] = ina3221_1.GetVoltageRAW(3);
        MatherBoardTelemtry[3] = ina3221_1.GetCurrentRAW(3);

        MatherBoardTelemtry[4] = ina3221_1.GetVoltageRAW(2);
        MatherBoardTelemtry[5] = ina3221_1.GetShuntVoltageRAW(2);

        MatherBoardTelemtry[6] = ina3221_2.GetVoltageRAW(1);
        MatherBoardTelemtry[7] = ina3221_2.GetCurrentRAW(1);

        MatherBoardTelemtry[8] = ina3221_2.GetVoltageRAW(3);
        MatherBoardTelemtry[9] = ina3221_2.GetCurrentRAW(3);

        MatherBoardTelemtry[10] = ina3221_2.GetVoltageRAW(2);
        MatherBoardTelemtry[11] = ina3221_2.GetShuntVoltageRAW(2);


        MatherBoardTelemtry[14] = ResetCounter;
        MatherBoardTelemtry[15] = (last_reset_time >> 16) & 0xFFFF;
        MatherBoardTelemtry[16] = last_reset_time & 0xFFFF;
        MatherBoardTelemtry[17] = 100;
        MatherBoardTelemtry[18] = (clock_get_hz(clk_sys) / 1000000);
        MatherBoardTelemtry[19] = cpu_temperature_read_raw();
//        uint32_t rtcUnix = rtc.dataTimeUnix();
//        printf("unixTime: %d\n", rtcUnix);

        MatherBoardTelemtry[22] = ADCReadRAW(0);
        MatherBoardTelemtry[23] = ADCReadRAW(1);
        MatherBoardTelemtry[24] = ADCReadRAW(2);

        MatherBoardTelemtry[25] = 256;
        MatherBoardTelemtry[26] = 256;

        MatherBoardTelemtry[27] = 256;
        MatherBoardTelemtry[28] = 256;

        getActiveParametrs(2, 29);
        printf("module 2 V:%d, C:%d\n", MatherBoardTelemtry[29], MatherBoardTelemtry[30]);
        getActiveParametrs(3, 31);
        getActiveParametrs(4, 33);
        getActiveParametrs(5, 35);
        getActiveParametrs(6, 37);
        getActiveParametrs(7, 39);
        getActiveParametrs(8, 41);
        getActiveParametrs(9, 43);
        getActiveParametrs(10, 45);

        MatherBoardTelemtry[47] = ina3221_solar_1.GetVoltageRAW(1);
        MatherBoardTelemtry[48] = ina3221_solar_1.GetCurrentRAW(1);

        MatherBoardTelemtry[49] = ina3221_solar_2.GetVoltageRAW(1);
        MatherBoardTelemtry[50] = ina3221_solar_2.GetCurrentRAW(1);

        MatherBoardTelemtry[51] = ina3221_solar_1.GetVoltageRAW(3);
        MatherBoardTelemtry[52] = ina3221_solar_1.GetCurrentRAW(3);

        MatherBoardTelemtry[53] = ina3221_solar_2.GetVoltageRAW(3);
        MatherBoardTelemtry[54] = ina3221_solar_2.GetCurrentRAW(3);

        MatherBoardTelemtry[55] = ina3221_solar_1.GetVoltageRAW(2);
        MatherBoardTelemtry[56] = ina3221_solar_1.GetCurrentRAW(2);

        MatherBoardTelemtry[57] = ina3221_solar_2.GetVoltageRAW(2);
        MatherBoardTelemtry[58] = ina3221_solar_2.GetCurrentRAW(2);

        MatherBoardTelemtry[59] = getEnambleStatus();
        MatherBoardTelemtry[60] = 256;
        MatherBoardTelemtry[61] = 256;


//        busy_wait_ms(1000);
    }
}

//        enableFlags1345.flagSlot1 = pca9554_1.getEnableFlag(1);
//        enableFlags1345.flagSlot2 = pca9554_1.getEnableFlag(8);
//        enableFlags1345.flagSlot3 = pca9554_1.getEnableFlag(9);
//        enableFlags1345.flagSlot4 = pca9554_1.getEnableFlag(10);
//
//        enableFlags6891.flagSlot1 =  pca9554_2.getEnableFlag(3);
//        enableFlags6891.flagSlot2 =  pca9554_2.getEnableFlag(4);
//        enableFlags6891.flagSlot3 =  pca9554_2.getEnableFlag(5);
//        enableFlags6891.flagSlot4 =  pca9554_2.getEnableFlag(6);
//
//        MatherBoardTelemtry[44] = enableFlags1345.data;
//        MatherBoardTelemtry[45] = enableFlags6891.data;

//ADC channel 0 GP26 VSolarBus
//ADC channel 1 GP27 VUsb
//ADC channel 2 GP28 VBus

// #Slots
//PCA: 0x3F pca9554_1
// #1-Radio1 Slot: 0x02
// #2-Battery1 state ON after Rbf remove Slot: 0x04
// #8-Coils Slot: 0x20
// #9-Radio2 Slot: 0x80
// #10-SolarSensors Slot: 0x10

//PCA: 0x38 pca9554_2
// #3-Reserve Slot: 0x01
// #4-Camera Slot: 0x02
// #5-GPS Slot: 0x04
// #6-Reserve Slot: 0x08
// #7-Battery2 state ON after Rbf remove Slot: 0x80


//i2c port scanner
//printf("\nI2C Bus Scan\n");
//printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
//
//for (int addr = 0; addr < (1 << 7); ++addr) {
//if (addr % 16 == 0) {
//printf("%02x ", addr);
//}
//
//int ret;
//uint8_t rxdata;
//if (reserved_addr(addr))
//ret = PICO_ERROR_GENERIC;
//else
//ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
//
//printf(ret < 0 ? "." : "@");
//printf(addr % 16 == 15 ? "\n" : "  ");
//}
//busy_wait_ms(1000);