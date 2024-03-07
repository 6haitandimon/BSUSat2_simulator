#include "soft_i2c.h"

uint8_t i2cAddress1;
GPIO_TypeDef *sda1;
GPIO_TypeDef *scl1;
uint32_t sdap1;
uint32_t sclp1;
GPIO_InitTypeDef SDAPinInit = {0};
GPIO_InitTypeDef SCLPinInit = {0};
GPIO_InitTypeDef SDAPinInput = {0};
HAL_StatusTypeDef result;

void I2C_Init(GPIO_TypeDef *SDAPort,uint32_t SDAPin,GPIO_TypeDef *SCLPort,uint32_t SCLPin)
{

    sda1=SDAPort;
    scl1=SCLPort;
    sdap1=SDAPin;
    sclp1=SCLPin;


    if(SDAPort==GPIOA || SCLPort==GPIOA  )
        __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(sda1, SDAPin|SCLPin, GPIO_PIN_RESET);

    SDAPinInit.Pin = SDAPin;
    SDAPinInit.Mode = GPIO_MODE_OUTPUT_PP;
    SDAPinInit.Pull = GPIO_NOPULL;
    SDAPinInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(sda1, &SDAPinInit);
    HAL_GPIO_WritePin(sda1, sdap1, GPIO_PIN_SET);

    SDAPinInput.Pin  = SDAPin;
    SDAPinInput.Mode = GPIO_MODE_INPUT;
    SDAPinInput.Pull = GPIO_NOPULL;

    SCLPinInit.Pin =SCLPin;
    SCLPinInit.Mode = GPIO_MODE_OUTPUT_PP;
    SCLPinInit.Pull = GPIO_NOPULL;
    SCLPinInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(scl1, &SCLPinInit);

    HAL_GPIO_WritePin(scl1, SCLPin, GPIO_PIN_SET);

}


void testOutput()
{
    scl1->BSRR = sclp1  ;
    sda1->BSRR = sdap1  ;

    HAL_Delay(1000);

    sda1->BSRR = (uint32_t)sdap1 << 16U ;
    scl1->BSRR = (uint32_t)sclp1 << 16U ;

}

void resetIO()
{
    changeSDAState(1) ;
    scl1->BSRR = sclp1  ;
    sda1->BSRR = sdap1  ;

}

void delay()
{
    volatile int i = 0;
    while(i < delayCount){
        i++;
    }
}


void startCondition()
{
    sda1->BSRR = (uint32_t)sdap1 << 16U ;
    delay();
    scl1->BSRR = (uint32_t)sclp1 << 16U ;
}

void stopCondition()
{

    changeSDAState(1);
    scl1->BSRR = sclp1;
    delay();
    sda1->BSRR = sdap1;
    delay();

}


void repeatedStartCondition()
{
    changeSDAState(1);
    sda1->BSRR = sdap1;
    delay();
    scl1->BSRR = sclp1;
    delay();
    sda1->BSRR = (uint32_t)sdap1 << 16U ;

}


void changeSDAState(int selection)
{


    if(selection==0)
    {
        SDAPinInit.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(sda1, &SDAPinInit);
    }
    else if(selection==1)
    {
        scl1->BSRR = (uint32_t)sclp1 << 16U ;
        SDAPinInit.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(sda1, &SDAPinInit);
        sda1->BSRR = (uint32_t)sdap1 << 16U ;
        scl1->BSRR = (uint32_t)sclp1 << 16U ;


    }

    for(int i=0;i<5;i++)
        delay();

}



bool waitACK(uint32_t timeOut)
{
    changeSDAState(0);

    int time=HAL_GetTick();
    bool ACK=0;

    scl1->BSRR = sclp1;
    delay();
    if(!(HAL_GPIO_ReadPin(sda1, sdap1)))
    {
        ACK=1;
        changeSDAState(1);

    }

    scl1->BSRR = (uint32_t)sclp1 << 16U ;
    delay();

    return ACK;
}

bool writeReadBitSet(uint8_t address,bool selection,int32_t timeOut)
{
    bool successfullACK=0;
    int writeAddress=0;
    if(selection)
        writeAddress=(address<<1) & 0xFE;
    else
        writeAddress=(address<<1) | 0x01;

    startCondition();

    for(int i=7;i>=0;i--)
    {
        if( (writeAddress>>i)&0x01)
            sda1->BSRR = sdap1;
        else
            sda1->BSRR = (uint32_t)sdap1 << 16U ;


        delay();
        scl1->BSRR = sclp1;
        delay();
        scl1->BSRR = (uint32_t)sclp1 << 16U ;


    }

    if(waitACK(timeOut))
        successfullACK=1;
    if(selection == 1)
        changeSDAState(1);
    return successfullACK;

}

bool writeBytes(uint8_t value,int32_t timeOut)
{
    bool successfullACK=0;

    scl1->BSRR = (uint32_t)sclp1 << 16U ;

    for(int i=7;i>=0;i--)
    {
        if( (value>>i)&0x01)
            sda1->BSRR = sdap1;
        else
            sda1->BSRR = (uint32_t)sdap1 << 16U ;

        delay();

        scl1->BSRR = sclp1;
        delay();
        scl1->BSRR = (uint32_t)sclp1 << 16U ;

    }

    if(waitACK(timeOut))
        successfullACK=1;

    return successfullACK;
}

uint8_t readLine()
{
    changeSDAState(0);
    uint8_t data=0;
    uint8_t counter=0;
    bool ACK=0;

    while(1)
    {

        scl1->BSRR = sclp1;
        delay();
        data |= HAL_GPIO_ReadPin(sda1, sdap1);
        counter++;

        scl1->BSRR = (uint32_t)sclp1 << 16U ;
        delay();


        if(counter==8)
            return data;

        data=data<<1;



    }



}

void sendACK(int select)
{
    changeSDAState(1);

    if(select == 1)
        sda1->BSRR = (uint32_t)sdap1 << 16U ;
    else
        sda1->BSRR = sdap1  ;

    delay();

    scl1->BSRR = sclp1;

    delay();

    scl1->BSRR = (uint32_t)sclp1 << 16U ;


}

HAL_StatusTypeDef I2C_DeviceReady(uint8_t deviceAddress)
{
    if(!writeReadBitSet(deviceAddress,1,500))
    {
        resetIO();
        return HAL_ERROR;
    }

    resetIO();
    return HAL_OK;

}
bool I2C_Write(uint8_t deviceAddress,uint8_t *sendData,uint8_t dataNumber,uint32_t timeOut)
{

    if(!writeReadBitSet(deviceAddress,1,500))
    {
        resetIO();
        return false;
    }

    for(int i=0;i<dataNumber;i++)
    {
        if(!writeBytes(sendData[i],timeOut))
        {
            resetIO();
            return false;
        }

    }

    stopCondition();
    return true;

}

bool I2C_Read (uint8_t deviceAddress,uint16_t registerAddress,uint8_t regLength,uint8_t *receiveBuffer,uint8_t dataNumber,uint32_t timeOut)
{

    if(!writeReadBitSet(deviceAddress,1,500))
    {
        resetIO();
        return false;
    }

    repeatedStartCondition();

    if(!writeReadBitSet(deviceAddress,0,500))
    {
        resetIO();
        return false;
    }

    for(int i=0;i<dataNumber;i++)
    {
        receiveBuffer[i]=readLine();
        if(i != dataNumber-1)
        {
            sendACK(1);
            changeSDAState(0);
        }

    }

    sendACK(0);
    stopCondition();
}