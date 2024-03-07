#ifndef BH1750_STM32_PORT_H
#define BH1750_STM32_PORT_H

#include "soft_i2c.h"
#include "stm32f0xx.h"

#define i2c_handleTypeDef sw_i2c_t

typedef enum
{
    OK       = 0x00U,
    Error    = 0x01U,
    BUSY     = 0x02U,
    TIMEOUT  = 0x03U
} StatusTypeDef;

/*
HAL_StatusTypeDef I2C_IsDeviceReady(sw_i2c_t *hi2c, uint8_t DevAddress, uint8_t Trials,uint8_t Timeout);
HAL_StatusTypeDef I2C_Master_Transmit(sw_i2c_t *hi2c, uint8_t DevAddress, uint8_t *pData,uint8_t Size, uint8_t Timeout);
HAL_StatusTypeDef I2C_Master_Receive(sw_i2c_t *hi2c, uint8_t DevAddress, uint8_t *pData,uint8_t Size, uint8_t Timeout);
*/

#endif //BH1750_STM32_PORT_H
