
#include "software_i2c.h"

/**
 * @brief Инициализация обеькта обработчика шины i2c
 * @return sw_i2c handler
 */
sw_i2cT SW_I2C_INIT(GPIO_TypeDef* sdaPort, u16 sdaPin, GPIO_TypeDef* sclPort, u16 sclPin) {
    sw_i2cT i2c_handler;
    GPIO_InitTypeDef temp = {0};

    i2c_handler.sdaPort = sdaPort;
    i2c_handler.sdaPin = sdaPin;
    i2c_handler.sclPort = sclPort;
    i2c_handler.sclPin = sclPin;

    //todo
    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(sdaPort, sdaPin|sclPin, GPIO_PIN_RESET);

    temp.Pin = sdaPin;
    temp.Mode = GPIO_MODE_OUTPUT_OD;
    temp.Pull = GPIO_NOPULL;// подтянуть в случаи отсутсвии резаков
    temp.Speed = GPIO_SPEED_FREQ_HIGH;// возможно стоит поставить GPIO_SPEED_FREQ_MEDIUM
    HAL_GPIO_Init(sdaPort, &temp);
    HAL_GPIO_WritePin(sdaPort, sdaPin, GPIO_PIN_SET);

    temp.Pin =sclPin;
    temp.Mode = GPIO_MODE_OUTPUT_OD;
    temp.Pull = GPIO_NOPULL;// подтянуть при отсутсвии резаков
    temp.Speed = GPIO_SPEED_FREQ_HIGH;// возможно стоит поставить GPIO_SPEED_FREQ_MEDIUM
    HAL_GPIO_Init(sclPort, &temp);
    HAL_GPIO_WritePin(sclPort, sclPin, GPIO_PIN_SET);
    i2c_handler.status = HAL_OK;

    return i2c_handler;
}
/**
 * @brief Отправка данных по шине i2c
 * @note Адрес назначения передавать без сдвига
 * @param [in,out] hi2c обработчик
 * @param [in] DevAddress адрес назначения
 * @param [in] pData данные на отправку
 * @param [in] Size размер данных
 * @param [in] Timeout время ожидания
 * @return HAL_OK отправка прошла успешна \n HAL_ERROR данные не отправлены
 */
HAL_StatusTypeDef I2C_MASTER_TRANSMIT(sw_i2cT *hi2c, u8 DevAddress, u8 *pData, u16 Size, u32 Timeout) {

    if(!writeReadBitSet(hi2c,DevAddress,1,500)){
        resetIO(hi2c);
        hi2c->status = HAL_ERROR;
        return hi2c->status;
    }

    for(int i=0;i<Size;i++){
        if(!writeBytes(hi2c,pData[i])){
            resetIO(hi2c);
            hi2c->status = HAL_ERROR;
            return hi2c->status;
        }
    }
    stopCondition(hi2c);
    return hi2c->status = HAL_OK;
}

/**
 * @note использовать по умолчанию defaultDelay
 * @param delayCount задержка
 */
void delay(u8 delayCount) {
    volatile int i = 0;
    while(i < delayCount){
        i++;
    }
}
/**
 * @brief Перключает состояния пина SDA из INPUT в OPEN_DRAIN и обратно
 * @param [in,out] hi2c handler i2c
 * @param [in] IO Input = 0 \n OpenDrain = 1
 */
void changeSDAstate(sw_i2cT * hi2c,u8 IO) {

    GPIO_InitTypeDef temp;
    temp.Pin = hi2c->sdaPin;
    temp.Pull = GPIO_NOPULL;
    temp.Speed = GPIO_SPEED_FREQ_HIGH;

    if(IO==IN){
        temp.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(hi2c->sdaPort, &temp);
    }
    else if(IO==OD){
        hi2c->sclPort->BSRR = hi2c->sclPin << 16U;

        temp.Mode = GPIO_MODE_OUTPUT_OD;
        HAL_GPIO_Init(hi2c->sdaPort, &temp);
        hi2c->sdaPort->BSRR = hi2c->sdaPin << 16U ;
        hi2c->sclPort->BSRR = (u32)hi2c->sclPort << 16U ;
    }

    delay(defaultDelay*5);
}
/**
 * @brief Создания стартовых сигналов перед началом обмена данными по шине I2C
 * @param [in,out] hi2c handler
 */
void startCondition(sw_i2cT * hi2c){
    //todo проверка на sda на выход
    hi2c->sclPort->BSRR = hi2c->sdaPin << 16U ;
    delay(defaultDelay);
    hi2c->sdaPort->BSRR = hi2c->sclPin << 16U ;
}
/**
 * @brief Остановка передачи через шину
 * @param [in,out] hi2c handler
 */
void stopCondition(sw_i2cT * hi2c){
    changeSDAstate(hi2c,OD);
    hi2c->sclPort->BSRR = hi2c->sdaPin;
    delay(defaultDelay);
    hi2c->sdaPort->BSRR = hi2c->sclPin;
    delay(defaultDelay);
}
/**
 * @brief Устанвока портов IO в изнчаальное состояние
 * @param [in,out] hi2c handler
 */
void resetIO(sw_i2cT *hi2c) {
    changeSDAstate(hi2c,OD) ;
    hi2c->sclPort->BSRR = hi2c->sclPin;
    hi2c->sdaPort->BSRR = hi2c->sdaPin;
}

/**
 * todo убрать bool selection \n selection ?
 * @brief
 * @param [in,out] hi2c handler i2c
 * @param [in] DevAddress адрес назначения без сдвига
 * @param [in] selection
 * @param [in] TimeOut время ожидания
 * @return True - успешно \n False - не успешно.
 */
bool writeReadBitSet(sw_i2cT *hi2c,u8 DevAddress,bool selection,u32 TimeOut) {

    startCondition(hi2c);

    bool ACK = 0;
    u8 writeAddress=0;

    //сдвиг адреса на 1 бит влево для передачи по шине i2c
    if(selection)
        writeAddress=(DevAddress<<1) & 0xFE; // установка младшего бита в 0
    else
        writeAddress=(DevAddress<<1) | 0x01; // установка младшего бита в 1

    for(int i=7;i>=0;i--){
        if( (writeAddress>>i)&0x01)
            hi2c->sdaPort->BSRR = hi2c->sdaPin;
        else
            hi2c->sdaPort->BSRR = hi2c->sdaPin << 16U ;

        delay(defaultDelay);
        hi2c->sclPort->BSRR = hi2c->sclPin;
        delay(defaultDelay);
        hi2c->sclPort->BSRR = hi2c->sclPin << 16U ;
    }

    if(waitACK(hi2c))
        ACK=1;

    if(selection == 1)
        changeSDAstate(hi2c,OD);

    return ACK;
}
/**
 * todo таймаут?
 * @brief Ожидание прихода подтерждения транзакции
 * @param [in,out]hi2c handler
 * @return True - Ack пришло ,\n False - Ack не пришло
 */
bool waitACK(sw_i2cT *hi2c){
    changeSDAstate(hi2c,IN);

    bool ACK=0;
    hi2c->sclPort->BSRR = hi2c->sclPin;
    delay(defaultDelay);

    if(!(HAL_GPIO_ReadPin(hi2c->sdaPort, hi2c->sdaPin))){
        ACK=1;
        changeSDAstate(hi2c,OD);
    }
    hi2c->sclPort->BSRR = hi2c->sclPin << 16U ;
    delay(defaultDelay);

    return ACK;
}

/**
 *
 * @param [in,out] hi2c handler
 * @param [in] value значение на запись
 * @return True - успешно \n False - не успешно
 */
bool writeBytes(sw_i2cT *hi2c, u8 value) {

    hi2c->sclPort->BSRR = hi2c->sclPin << 16U ;

    for(int i=7;i>=0;i--)
    {
        if( (value>>i)&0x01)
            hi2c->sdaPort->BSRR = hi2c->sdaPin;
        else
            hi2c->sdaPort->BSRR = (u32)hi2c->sdaPin << 16U ;

        delay(defaultDelay);

        hi2c->sclPort->BSRR = hi2c->sclPin;
        delay(defaultDelay);
        hi2c->sclPort->BSRR = hi2c->sclPin << 16U ;

    }
    if(waitACK(hi2c))
        return true;

    return false;
}
/**
 * //todo подумать на возвратом ошибок
 * @brief Функция приема по шине i2c
 * @note Адрес назначения передавать без сдвига
 * @param hi2c handler
 * @param DevAddress адрес назначения
 * @param pData - буфер прием
 * @param size - размер данных
 * @param Timeout - время ожидааня
 * @return HAL_OK данные получены без ошибок \n
 */
HAL_StatusTypeDef I2C_MASTER_RECEIVE(sw_i2cT *hi2c, u8 DevAddress, u8 *pData, u16 size, u32 Timeout) {
    if(!writeReadBitSet(hi2c,DevAddress,1,500)){
        resetIO(hi2c);
        return HAL_ERROR;
    }

    repeatedStartCondition(hi2c);

    if(!writeReadBitSet(hi2c,DevAddress,0,500)){
        resetIO(hi2c);
        return HAL_ERROR;
    }

    for(int i=0;i<size;i++){
        pData[i]=readLine(hi2c);
        if(i != size-1){
            sendACK(hi2c,1);
            changeSDAstate(hi2c,IN);
        }
    }

    sendACK(hi2c,0);
    stopCondition(hi2c);
    return HAL_OK;
}
/**
 * @param hi2c handler
 */
void repeatedStartCondition(sw_i2cT *hi2c) {
    changeSDAstate(hi2c,OD);
    hi2c->sdaPort->BSRR = hi2c->sdaPin;
    delay(defaultDelay);
    hi2c->sclPort->BSRR = hi2c->sclPin;
    delay(defaultDelay);
    hi2c->sdaPort->BSRR = hi2c->sdaPin << 16U ;
}

u8 readLine(sw_i2cT *hi2c) {
    changeSDAstate(hi2c,IN);
    u8 data = 0;

    for(u8 i = 0; i < 8 ; i++){
        hi2c->sclPort->BSRR = hi2c->sclPin;
        delay(defaultDelay);
        data |= HAL_GPIO_ReadPin(hi2c->sdaPort, hi2c->sdaPin);

        hi2c->sclPort->BSRR = hi2c->sclPin << 16U ;
        delay(defaultDelay);
        if(i!=8)
            data<<=1;
    }
    return data;
}

void sendACK(sw_i2cT *hi2c,uint8_t select) {
    changeSDAstate(hi2c,OD);
    if(select == 1)
        hi2c->sdaPort->BSRR = hi2c->sdaPin << 16U;
    else
        hi2c->sdaPort->BSRR = hi2c->sdaPin;

    delay(defaultDelay);

    hi2c->sclPort->BSRR = hi2c->sclPin;

    delay(defaultDelay);

    hi2c->sclPort->BSRR = hi2c->sclPin << 16U;

}
/**
 *
 * @param [in] hi2c handler
 * @param [in] DevAddress  адресс назначения
 * @return HAL_OK устройство готово к работе \n HAL_BUSY устройство занято
 */
HAL_StatusTypeDef IsDeviceReady(sw_i2cT *hi2c,uint8_t DevAddress) {
    if(!writeReadBitSet(hi2c,DevAddress,1,10)){
        resetIO(hi2c);
        return HAL_BUSY;
    }
    return HAL_OK;
}

