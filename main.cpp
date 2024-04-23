#include "PCA9554.h"
#include "mcp2515.h"
#include "ina3221.h"
#include "INA219.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "interrupt.h"
#include "math.h"

#define INA3221_SHUNT_T_RESISTANCE 270.0

MCP2515 can0 = MCP2515( spi0, 17, 19, 16, 18);

INA3221::INA3221 ina3221_1 = INA3221::INA3221(i2c0, 20, 21, 0x42, 4.2, 3.3, 0.2);
INA3221::INA3221 ina3221_2 = INA3221::INA3221(i2c0, 20, 21, 0x43, 4.2, 3.3, 0.2);

PCA9554 pca9554_1 = PCA9554(i2c0, 21, 20, 0x3F, 0x40);
PCA9554 pca9554_2 = PCA9554(i2c0, 21, 20, 0x38, 0x70);
INA219::INA219 ina219_slot = INA219::INA219(i2c0, 20, 21,  0x44, 4.2, 1.0, 3.2, 0.1, 3291);
float convertionFactor = 3.009 / (4095.0) * (5.0 / 3.0);

double A = 0.8781625571e-3;
double B = 2.531972392e-4;
double C = 1.840753501e-7;

float MatherBoardTelemtry[13];


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

int main() {
    stdio_init_all();

    ADCInit();

    can0.reset();
    can0.setBitrate(CAN_1000KBPS, MCP_16MHZ);
    INTERRUPT::MCPInterruptSetup();
    can0.setNormalMode();



    while (true) {

        MatherBoardTelemtry[0] = ADCRead(1);
        MatherBoardTelemtry[1] = ADCRead(2);

        MatherBoardTelemtry[2] = ina3221_1.GetVoltage(1);
        MatherBoardTelemtry[3] = ina3221_1.GetCurrent(1);

        MatherBoardTelemtry[4] = ina3221_1.GetVoltage(3);
        MatherBoardTelemtry[5] = ina3221_1.GetCurrent(3);

        MatherBoardTelemtry[6] = ina3221_2.GetVoltage(1);
        MatherBoardTelemtry[7] = ina3221_2.GetCurrent(1);

        MatherBoardTelemtry[8] = ina3221_2.GetVoltage(3);
        MatherBoardTelemtry[9] = ina3221_2.GetCurrent(3);

        float TBat_1 = ina3221_1.GetVoltage(2);
        float TBatShunt_1 = ina3221_1.GetShuntVoltage(2);

        float TBat_2 = ina3221_2.GetVoltage(2);
        float TBatShunt_2 = ina3221_2.GetShuntVoltage(2);

        float RTermistorBat_1 = TBat_1 / (TBatShunt_1 / INA3221_SHUNT_T_RESISTANCE);
        float RTermistorBat_2 = TBat_2 / (TBatShunt_2 / INA3221_SHUNT_T_RESISTANCE);

        MatherBoardTelemtry[10] = 1.0 / (A + B * log(RTermistorBat_1) + C * pow(log(RTermistorBat_1), 3));
        MatherBoardTelemtry[11] = 1.0 / (A + B * log(RTermistorBat_2) + C * pow(log(RTermistorBat_2), 3));

        MatherBoardTelemtry[10] -= 273.15;
        MatherBoardTelemtry[11] -= 273.15;

    }
}

//ADC channel 0 GP26 VSolarBus
//ADC channel 1 GP27 VUsb
//ADC channel 2 GP28 VBus

// #Slots
//PCA: 0x3F
// #1-Radio1 Slot: 0x02
// #2-Battery1 state ON after Rbf remove Slot: 0x04
// #8-Coils Slot: 0x20
// #9-Radio2 Slot: 0x80
// #10-SolarSensors Slot: 0x10

//PCA: 0x38
// #3-Reserve Slot: 0x01
// #4-Camera Slot: 0x02
// #5-GPS Slot: 0x04
// #6-Reserve Slot: 0x08
// #7-Battery2 state ON after Rbf remove Slot: 0x80
