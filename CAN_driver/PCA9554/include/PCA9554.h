#pragma ONCE
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "PCA9554_reg.h"
#include <stdint.h>
#include <stdio.h>

class PCA9554{
    private:
        uint8_t I2C_num;
        uint8_t address;
        uint8_t config;
        uint8_t SCL;
        uint8_t SDA;
        uint8_t slotBit[11];
    public:
        PCA9554(uint8_t I2C_num, uint8_t SCL, uint8_t SDA, uint8_t address, uint8_t config);
        bool configuration(uint8_t config);
        void slotAdd(uint8_t slotNumber, uint8_t slotAddress);
        bool enableSlot(uint8_t slotNumber);
        bool disableSlot(uint8_t slotNumber);
        bool disableAllSlots();

    private:
        bool write(uint8_t registr, uint8_t data);
        uint8_t read(uint8_t registr);



};
