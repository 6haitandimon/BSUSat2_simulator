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

float convertionFactor = 3.009 / (4095.0) * (5.0 / 3.0);


double A = 0.8781625571e-3;
double B = 2.531972392e-4;
double C = 1.840753501e-7;

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
    can0.setBitrate(CAN_200KBPS, MCP_8MHZ);
    can0.setNormalMode();
    MCPInterruptSetup();
    while (true) {

        float VUSB = ADCRead(1);
        float VBUS = ADCRead(2);

        float vBat1_1 = ina3221_1.GetVoltage(1);
        float cBat1_1 = ina3221_1.GetCurrent(1);

        float vBat1_2 = ina3221_1.GetVoltage(3);
        float cBat1_2 = ina3221_1.GetCurrent(3);

        float vBat2_1 = ina3221_2.GetVoltage(1);
        float cBat2_1 = ina3221_2.GetCurrent(1);

        float vBat2_2 = ina3221_2.GetVoltage(3);
        float cBat2_2 = ina3221_2.GetCurrent(3);

        float TBat_1 = ina3221_1.GetVoltage(2);
        float TBatShunt_1 = ina3221_1.GetShuntVoltage(2);

        float TBat_2 = ina3221_2.GetVoltage(2);
        float TBatShunt_2 = ina3221_2.GetShuntVoltage(2);

        float RTermistorBat_1 = TBat_1 / (TBatShunt_1 / INA3221_SHUNT_T_RESISTANCE);
        float RTermistorBat_2 = TBat_2 / (TBatShunt_2 / INA3221_SHUNT_T_RESISTANCE);

        float TempBat1 = 1.0 / (A + B * log(RTermistorBat_1) + C * pow(log(RTermistorBat_1), 3));
        float TempBat2 = 1.0 / (A + B * log(RTermistorBat_2) + C * pow(log(RTermistorBat_2), 3));

        TempBat1 -= 273.15;
        TempBat2 -= 273.15;

        printf("VBUS: %f\nVUSB: %f\n\n", VBUS, VUSB);

        printf("vBat1_1: %f\ncBat1_1: %f\n\n", vBat1_1, cBat1_1);
        printf("vBat1_2: %f\ncBat1_2: %f\n\nTBat: %f\n\n\n", vBat1_2, cBat1_2, TempBat1);

        printf("vBat2_1: %f\ncBat2_1: %f\n\n", vBat2_1, cBat2_1);
        printf("vBat2_2: %f\ncBat2_2: %f\n\nTBat: %f\n\n\n", vBat2_2, cBat2_2, TempBat2);
        sleep_ms(150);

    }
}

//ADC channel 0 GP26 VSolarBus
//ADC channel 1 GP27 VUsb
//ADC channel 2 GP28 VBus

// #Slots
// #1-Radio1
// #2-Battery1 state ON after Rbf remove
// #3-Reserve
// #4-Camera
// #5-GPS
// #6-Reserve
// #7-Battery2 state ON after Rbf remove
// #8-Coils
// #9-Radio2
// #10-SolarSensors