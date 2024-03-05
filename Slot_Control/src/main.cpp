#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "PCA9554.h"
#include "stdlib.h"

#define SCL 21
#define SDA 20

#define SCL_slave 7
#define SDA_slave 6

#define I2C0_SLAVE_ADDR 0x41

union floatByte{
    float data;
    struct
    {
        uint8_t byte1;
        uint8_t byte2;
        uint8_t byte3;
        uint8_t byte4;
    };
};

float convertionFactor = 3.009 / (4095.0) * (5.0 / 3.0);
uint8_t ram_addr;
uint8_t ram[256];

void i2c1_irq_handler() {
    uint32_t status = i2c1_hw->intr_stat;
    uint32_t value;

    if (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        value = i2c1_hw->data_cmd;

        if (value & I2C_IC_DATA_CMD_FIRST_DATA_BYTE_BITS) {
            ram_addr = (uint8_t)(value & I2C_IC_DATA_CMD_BITS);
        }
        else {
            ram[ram_addr++] = (uint8_t)(value & I2C_IC_DATA_CMD_BITS);
        }
    }

    if (status & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
        i2c1_hw->data_cmd = (uint32_t)(ram[ram_addr++]);

        i2c1_hw->clr_rd_req;
    }
}

void ADCInit(){
    adc_init();
    adc_gpio_init(28); //bat
    adc_gpio_init(27); //usb
    adc_gpio_init(26); //solar
    return;
}

void I2CInit(){
    i2c_init(i2c0, 400000);
    i2c_init(i2c1, 400000);

    i2c_set_slave_mode(i2c1, true, I2C0_SLAVE_ADDR);
  
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_set_function(SDA_slave, GPIO_FUNC_I2C);
    gpio_set_function(SCL_slave, GPIO_FUNC_I2C);
  
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    gpio_pull_up(SDA_slave);
    gpio_pull_up(SCL_slave);

    i2c1_hw->intr_mask = I2C_IC_INTR_MASK_M_RD_REQ_BITS
        | I2C_IC_INTR_MASK_M_RX_FULL_BITS;

    irq_set_exclusive_handler(I2C1_IRQ, i2c1_irq_handler);
    
    irq_set_enabled(I2C1_IRQ, true);
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




int main(){
    stdio_init_all();

    ADCInit();
    I2CInit();

    while(true){
        printf("Vusb: %f, VBus: %f, VSolarBus: %f\n\n",  ADCRead(2), ADCRead(1), ADCRead(0));

        sleep_ms(2000);
    }

    return 0;
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