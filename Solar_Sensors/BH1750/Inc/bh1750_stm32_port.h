#ifndef BH1750_STM32_PORT_H
#define BH1750_STM32_PORT_H

#include "main.h"
#include "i2c.h"
#include "software_i2c.h"

#define sw_i2c_t sw_i2cT

typedef enum
{
    OK       = 0x00U,
    Error    = 0x01U,
    BUSY     = 0x02U,
    TIMEOUT  = 0x03U
} StatusTypeDef;


HAL_StatusTypeDef I2C_IsDeviceReady(sw_i2c_t *hi2c, uint16_t DevAddress, uint8_t Trials,uint32_t Timeout);
HAL_StatusTypeDef I2C_Master_Transmit(sw_i2c_t *hi2c, uint16_t DevAddress, uint8_t *pData,uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef I2C_Master_Receive(sw_i2c_t *hi2c, uint16_t DevAddress, uint8_t *pData,uint16_t Size, uint32_t Timeout);


#endif //BH1750_STM32_PORT_H
