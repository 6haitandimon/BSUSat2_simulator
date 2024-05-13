#include "PCA9554.h"
#include "mcp2515.h"
#include "ina3221.h"
#include "INA219.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "interrupt.h"
#include "math.h"

#define INA3221_SHUNT_T_RESISTANCE 270.0

union matherBoardEnambleSlotFlags{
    uint32_t data;
    struct{
        bool flagSlot1;
        bool flagSlot2;
        bool flagSlot3;
        bool flagSlot4;
    };
};


MCP2515 can0 = MCP2515(spi0, 17, 19, 16, 18);

INA3221::INA3221 ina3221_1 = INA3221::INA3221(i2c0, 20, 21, 0x42, 4.2, 3.3, 0.2);
INA3221::INA3221 ina3221_2 = INA3221::INA3221(i2c0, 20, 21, 0x43, 4.2, 3.3, 0.2);

INA3221::INA3221 ina3221_solar_1 = INA3221::INA3221(i2c0, 20, 21, 0x40, 4.2, 3.3, 0.2);
INA3221::INA3221 ina3221_solar_2 = INA3221::INA3221(i2c0, 20, 21, 0x41, 4.2, 3.3, 0.2);

PCA9554 pca9554_1 = PCA9554(i2c0, 21, 20, 0x3F, 0x40);
PCA9554 pca9554_2 = PCA9554(i2c0, 21, 20, 0x38, 0x70);

INA219::INA219 ina219_slot = INA219::INA219(i2c0, 20, 21,  0x44, 0.150, 4.2, 3.2, 0.1, 4096);

float convertionFactor = 3.009 / (4095.0) * (5.0 / 3.0);
double A = 0.8781625571e-3;

double B = 2.531972392e-4;
double C = 1.840753501e-7;

float MatherBoardTelemtry[47];

matherBoardEnambleSlotFlags enableFlags1345;
matherBoardEnambleSlotFlags enableFlags6891;


void ADCInit() {
    adc_init();
    adc_gpio_init(28); //bat
    adc_gpio_init(27); //usb
    adc_gpio_init(26); //solar
    return;
}

uint16_t adc_config_and_read_u16(uint32_t channel) {
    adc_select_input(channel);
    uint32_t raw = adc_read();
    const uint32_t bits = 12;

    return raw << (16 - bits) | raw >> (2 * bits - 16);
}

float ADCRead(uint32_t channel){
    float value = (adc_config_and_read_u16(channel) >> 4) * convertionFactor;
    return value;
}

void slotInit(){
    pca9554_1.slotAdd(1, 0x02);
    pca9554_1.slotAdd(8, 0x20);
    pca9554_1.slotAdd(9, 0x80);
    pca9554_1.slotAdd(10, 0x10);

    pca9554_2.slotAdd(3, 0x01);
    pca9554_2.slotAdd(4, 0x02);
    pca9554_2.slotAdd(5, 0x04);
    pca9554_2.slotAdd(6, 0x08);

    return;
}

void getActiveParametrs(uint8_t slotNumber, uint8_t dataNumber){
    ina219_slot.changeAddres(0x44 + slotNumber - 1);
    MatherBoardTelemtry[dataNumber++] = ina219_slot.get_voltage();
    MatherBoardTelemtry[dataNumber] = ina219_slot.get_current();

    return;
}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main() {
    stdio_init_all();

    ADCInit();
    slotInit();
    can0.reset();
    can0.setBitrate(CAN_1000KBPS, MCP_16MHZ);
    INTERRUPT::MCPInterruptSetup();
    can0.setNormalMode();

    pca9554_1.enableSlot(9);
    pca9554_2.enableSlot(3);



    while (true) {

        printf("\nI2C Bus Scan\n");
        printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

        for (int addr = 0; addr < (1 << 7); ++addr) {
            if (addr % 16 == 0) {
                printf("%02x ", addr);
            }

            int ret;
            uint8_t rxdata;
            if (reserved_addr(addr))
                ret = PICO_ERROR_GENERIC;
            else
                ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

            printf(ret < 0 ? "." : "@");
            printf(addr % 16 == 15 ? "\n" : "  ");
        }
        busy_wait_ms(1000);

//        MatherBoardTelemtry[0] = ina3221_1.GetVoltage(1);
//        MatherBoardTelemtry[1] = ina3221_1.GetCurrent(1);
//
//        MatherBoardTelemtry[2] = ina3221_1.GetVoltage(3);
//        MatherBoardTelemtry[3] = ina3221_1.GetCurrent(3);
//
//        MatherBoardTelemtry[5] = ina3221_2.GetVoltage(1);
//        MatherBoardTelemtry[6] = ina3221_2.GetCurrent(1);
//
//        MatherBoardTelemtry[7] = ina3221_2.GetVoltage(3);
//        MatherBoardTelemtry[8] = ina3221_2.GetCurrent(3);
//
//
//
//        MatherBoardTelemtry[10] = ina3221_solar_1.GetVoltage(1);
//        MatherBoardTelemtry[11] = ina3221_solar_1.GetCurrent(1) * (-1);
//
//        MatherBoardTelemtry[12] = ina3221_solar_2.GetVoltage(1);
//        MatherBoardTelemtry[13] = ina3221_solar_2.GetCurrent(1) * (-1);
//
//        MatherBoardTelemtry[14] = ina3221_solar_1.GetVoltage(3);
//        MatherBoardTelemtry[15] = ina3221_solar_1.GetCurrent(3) * (-1);
//
//        MatherBoardTelemtry[16] = ina3221_solar_2.GetVoltage(3);
//        MatherBoardTelemtry[17] = ina3221_solar_2.GetCurrent(3) * (-1);
//
//        MatherBoardTelemtry[18] = ADCRead(0);
//        MatherBoardTelemtry[19] = ADCRead(1);
//        MatherBoardTelemtry[20] = ADCRead(2);
//
////        getActiveParametrs(1, 28);
////        ina219_slot.changeAddres(0x4e);
////        MatherBoardTelemtry[28]  =  ina219_slot.get_voltage();
////        MatherBoardTelemtry[29]  =  ina219_slot.get_current();
//        getActiveParametrs(2, 28);
//        getActiveParametrs(3, 30);
//        getActiveParametrs(4, 32);
//        getActiveParametrs(5, 34);
//        getActiveParametrs(6, 36);
//        getActiveParametrs(8, 38);
//        getActiveParametrs(9, 40);
//        getActiveParametrs(10, 42);
//
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
//
    }
}

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
