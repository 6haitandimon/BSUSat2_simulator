<h1 align="center">Hi, I'm <a href="https://github.com/st33ng" target="_blank">steeng</a> 
<img src="https://github.com/blackcater/blackcater/raw/main/images/Hi.gif" height="32"/></h1>

# Software I2C
## How to use
### Receive Data
```c
#include "software_i2c.h"

int main(void){
    // init struct to PA1(SDA),PA0(SCL)
    sw_i2cT hi2c = SW_I2C_INIT(GPIOA,GPIO_PIN_1,GPIOA,GPIO_PIN_0); 
    uint8_t *pdata[2];
    DevAddress = 0x01;
    // read data from slaver
    // DevAddress is address without shift 
    I2C_MASTER_RECEIVE(&hi2c,DevAddress,pData,2,10);
}
```
### Transmit Data
```c
#include "software_i2c.h"

int main(void){
    // init struct to PA1(SDA),PA0(SCL)
    sw_i2cT hi2c = SW_I2C_INIT(GPIOA,GPIO_PIN_1,GPIOA,GPIO_PIN_0); 
    uint8_t *pdata[1] = {2};
    DevAddress = 0x01;
    // send data from master
    // DevAddress is address without shift 
    I2C_MASTER_TRANSMIT(&hi2c,DevAddress,pData,1,10)
}
```
### Check the device is ready for operation

```c
#include "software_i2c.h"

int main(void){
    sw_i2cT hi2c = SW_I2C_INIT(GPIOA,GPIO_PIN_1,GPIOA,GPIO_PIN_0);
    DevAddress = 0x01;
    //check
    IsDeviceReady(&hi2c,DevAddress);
}
```