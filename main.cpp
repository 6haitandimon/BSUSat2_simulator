#include "PCA9554.h"
#include "mcp2515.h"
#include "ina3221.h"
#include "INA219.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "interrupt.h"

//int main() {
//    stdio_init_all();
//    MCP2515 can0 = MCP2515( spi0, 17, 19, 16, 18);
//    can0.reset();
//    can0.setBitrate(CAN_1000KBPS, MCP_16MHZ);
//    can0.setNormalMode();
//    can_frame tx;
//    tx.can_id = 10;
//    tx.can_dlc = 8;
//    tx.data[0] = 1;
//    tx.data[1] = 1;
//    tx.data[2] = 1;
//    tx.data[3] = 1;
//    tx.data[4] = 1;
//    tx.data[5] = 1;
//    tx.data[6] = 1;
//    tx.data[7] = 1;
//    while (true) {
//        can0.sendMessage(&tx);
//        sleep_ms(1000);
//    }
//}


MCP2515 can0 = MCP2515( spi0, 17, 19, 16, 18);

void ADCInit() {
    adc_init();
    adc_gpio_init(28); //bat
    adc_gpio_init(27); //usb
    adc_gpio_init(26); //solar
    return;
}

int main() {
    stdio_init_all();
    ADCInit();
    can0.reset();
    can0.setBitrate(CAN_200KBPS, MCP_8MHZ);
    can0.setNormalMode();
    MCPInterruptSetup();
    while (true) {

    }
}

//ADC channel 0 GP26 VSolarBus
//ADC channel 1 GP27 VBus
//ADC channel 2 GP28 VUsb

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