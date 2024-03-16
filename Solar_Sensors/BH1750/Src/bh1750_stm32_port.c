#include "bh1750_stm32_port.h"


HAL_StatusTypeDef I2C_IsDeviceReady(sw_i2c_t *hi2c, uint16_t DevAddress, uint8_t Trials,uint32_t Timeout){
    return IsDeviceReady(hi2c,DevAddress);
}

HAL_StatusTypeDef I2C_Master_Transmit(sw_i2c_t *hi2c, uint16_t DevAddress, uint8_t *pData,uint16_t Size, uint32_t Timeout){
    return I2C_MASTER_TRANSMIT(hi2c,DevAddress>>1,pData,Size,Timeout);
}

HAL_StatusTypeDef I2C_Master_Receive(sw_i2c_t *hi2c, uint16_t DevAddress, uint8_t *pData,uint16_t Size, uint32_t Timeout){
    return I2C_MASTER_RECEIVE(hi2c,DevAddress>>1,pData,Size,Timeout);
}

void _delay(uint32_t __ms) {
    HAL_Delay(__ms);
}
