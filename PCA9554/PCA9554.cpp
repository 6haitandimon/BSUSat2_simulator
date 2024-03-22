#include "PCA9554.h"

PCA9554::PCA9554(uint8_t I2C_num, uint8_t SCL, uint8_t SDA, uint8_t address, uint8_t config) :
        I2C_num(I2C_num), SCL(SCL), 
        SDA(SDA), address(address), 
        config(config){
            if(I2C_num == 0)
                i2c_init(i2c0, 400000);
            else
            i2c_init(i2c1, 400000);
            gpio_set_function(SDA, GPIO_FUNC_I2C);
            gpio_set_function(SCL, GPIO_FUNC_I2C);
            gpio_pull_up(SCL);
            gpio_pull_up(SDA);
            
            write(PCA9554_REG_CONFIG, config);
        }

bool PCA9554::configuration(uint8_t config){
    this->config = config;
    if(write(PCA9554_REG_CONFIG, this->config)){
        printf("config update successful\n");
        return 1;
    }
    else{
        printf("config update error\n");
        return 0;
    }
}

bool PCA9554::disableAllSlots(){
    if(write(PCA9554_REG_OUTPUT_PORT, 0)){
        printf("All slots enable successful\n");
        return 1;
    }else{
        printf("All slots enable error\n");
        return 0;
    }
}

void PCA9554::slotAdd(uint8_t slotNumber, uint8_t slotAddress){
    this->slotBit[slotNumber] = slotAddress;
}

bool PCA9554::enableSlot(uint8_t slotNumber){
    if(write(PCA9554_REG_OUTPUT_PORT, this->slotBit[slotNumber])){
        printf("Slot enable successful\n");
        return 1;
    }else{
        printf("slot enable error\n");
        return 0;
    }
}

bool PCA9554::disableSlot(uint8_t slotNumber){
    uint8_t configRegStatus = read(PCA9554_REG_INPUT_PORT);
    //for debug
    printf("config reg status: %d\n", configRegStatus);
    printf("Slot bit status: %d\n", configRegStatus ^ this->slotBit[slotNumber]);

    if(write(PCA9554_REG_OUTPUT_PORT, configRegStatus ^ this->slotBit[slotNumber])){
        printf("Slot enable successful\n");
        return 1;
    }else{
        printf("slot enable error\n");
        return 0;
    }
    // if(write(PCA9554_REG_OUTPUT_PORT, this->slotBit[slotNumber])){
    //     printf("Slot enable successful")
    //     return 1;
    // }else{
    //     printf("slot enable error")
    //     return 0;
    // }
}

bool PCA9554::write(uint8_t registr, uint8_t data){
    uint8_t writeData[2] = {registr, data};
    uint8_t ret = 0;
    if(I2C_num == 0){
        ret = i2c_write_blocking(i2c0, this->address, writeData, 2, false);
    }
    else {
        ret = i2c_write_blocking(i2c1, this->address, writeData, 2, false);
    }

    if(ret < 0)  return 0;
    else return 1;
}

uint8_t PCA9554::read(uint8_t registr){
    uint8_t readData[2];
    uint8_t ret = 0;
    if(I2C_num == 0){
        ret = i2c_write_blocking(i2c0, this->address, &registr, 1, false);
        if(ret < 0)
            printf("Error write register\n");

        ret = i2c_read_blocking(i2c0, this->address, readData, 1, false);
        if(ret < 0)
            printf("Failed to read register value\n");
    }
    else {
        ret = i2c_write_blocking(i2c1, this->address, &registr, 1, false);
        if(ret < 0)
            printf("Error write register\n");

        ret = i2c_read_blocking(i2c0, this->address, readData, 1, false);
        if(ret < 0)
            printf("Failed to read register value\n");
    }
    return readData[0];
}