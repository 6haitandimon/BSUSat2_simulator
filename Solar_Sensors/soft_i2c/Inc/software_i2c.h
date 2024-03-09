#ifndef SOFTWARE_I2C_H
#define SOFTWARE_I2C_H

#include "stm32_port_i2c.h"
#define bool u8
#define true 1
#define false 0

#define defaultDelay 20

typedef struct sw_i2c{
    GPIO_TypeDef *sdaPort;
    GPIO_TypeDef *sclPort;
    u16 sdaPin;
    u16 sclPin;
    HAL_StatusTypeDef status;
}sw_i2cT;

enum IO{
    IN = 0,
    OD
};

sw_i2cT SW_I2C_INIT(GPIO_TypeDef* sdaPort,u16 sdaPin, GPIO_TypeDef* sclPort,u16 sclPin);
HAL_StatusTypeDef I2C_MASTER_TRANSMIT(sw_i2cT* hi2c,u8 DevAddress,u8 *pData,u16 Size,u32 Timeout);
HAL_StatusTypeDef I2C_MASTER_RECEIVE(sw_i2cT * hi2c,u8 DevAddress,u8 *pData,u16 size,u32 Timeout);
HAL_StatusTypeDef IsDeviceReady(sw_i2cT *hi2c,u8 DevAddress);
bool writeReadBitSet(sw_i2cT *hi2c,u8 DevAddress,bool selection,u32 Timeout);
bool writeBytes(sw_i2cT *hi2c,u8 value);
bool waitACK(sw_i2cT *hi2c);
u8 readLine(sw_i2cT * hi2c);
void sendACK(sw_i2cT *hi2c,u8 select);
void repeatedStartCondition(sw_i2cT *hi2c);
void delay(u8 delayCount );
void changeSDAstate(sw_i2cT * hi2c,u8 IO);
void stopCondition(sw_i2cT * hi2c);
void startCondition(sw_i2cT * hi2c);
void resetIO(sw_i2cT * hi2c);
#endif //SOFTWARE_I2C_H
