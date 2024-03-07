/**
**************************************************************************
* @file           : BH1750.cpp
* @brief          : Драйвер для работы с датчиком интенсивности света BH1750.
* @author         : steeng
**************************************************************************
*/
#include "BH1750.h"
/**
 * @brief Функция иниициализация обьекта датчик
 * @param [in] i2c_handle Обработчик шины i2c
 * @param [in] ADDR_GND ADDR подключен на землю или нет
 * @return Возвращает инициализированный Обьект сенсора BH1750_t
 */
BH1750_t BH1750_Init(i2c_handleTypeDef* i2c_handle,bool ADDR_GND){
    BH1750_t obj;

    obj.address_RD = ADDR_GND ? GND_ADDR_RD : NO_GND_ADDR_RD;
    obj.address_WR = ADDR_GND ? GND_ADDR_WR : NO_GND_ADDR_WR;
    obj.i2c_handle = i2c_handle;

    obj.status = BH1750_ON(&obj); //включаем его
    return obj;
}
/**
 * @brief Проверяет наличие или готовность устройства перед выполнением операций чтения или записи данных через шину I2C.
 * @param [in] obj Обьект для общения с датчиком
 * @return возвращает состояния датчика StatusTypeDef
 */
StatusTypeDef ChechStatus(const BH1750_t* obj){
    return I2C_IsDeviceReady(obj->i2c_handle,obj->address_RD,
                                 4,100); // 4 -// количество попыток
}
/**
 * @brief Передает команду на исполнение в датчик
 * @param [in] obj Обьект для общения с датчиком
 * @param [in] cmd команда на исполнение
 * @return возвращает состояния датчика после исполнения команды StatusTypeDef
 */
StatusTypeDef BH1750_WR(const BH1750_t * obj,uint8_t cmd){
    if(I2C_Master_Transmit(obj->i2c_handle,       // i2c handler
                               obj->address_WR  // адрес на запись
                               ,&cmd,               // команда на исполнение
                               1,10)!= OK) {return Error;};

    return OK;
}
/**
 * @brief Считывает значение с датчика
 * @param [in,out] obj Обьект для общения с датчиком
 * @return возвращает состояния датчика после чтения данных StatusTypeDef
 */
StatusTypeDef BH1750_RD(BH1750_t  *obj){
    if(I2C_Master_Receive(obj->i2c_handle,obj->address_RD,
                              obj->buffer,2,10) != OK) {return Error;};
    return OK;
}
/**
 * @brief Функция для "включения" датчика
 * @param [in] obj Обьект для общения с датчиком
 * @return возвращает состояния датчика после включения StatusTypeDef.
 */
StatusTypeDef BH1750_ON(const BH1750_t *obj){
    StatusTypeDef status;

    if ((status = BH1750_WR(obj, POWER_ON)) != OK ||
    (status = BH1750_WR(obj, RESET_)) != OK ||
    (status = BH1750_WR(obj, H_RES_MODE)) != OK) {
        return status;
    }
    return OK;
}
/**
 * @brief Считывает значения с датчика в lx
 * @note obj->value изменяет значение ,можно получить из него lx
 * @note
 * @param [in,out] obj Обьект для общения с датчиком
 * @return возвращает значение в lx uint16_t
 */
/** example from datasheet \n
    *How to calculate when the data High Byte is "10000011" and Low Byte is "10010000" (215+29+28+27+24 )/1.2 = 28067[lx]\n
    *How to calculate when the data High Byte is "00000001" and Low Byte is "00010000" (28+24 )/1.2 = 227[lx]
    * */
float BH1750_get_value(BH1750_t* obj){
    BH1750_RD(obj);

    obj->value =  ( (obj->buffer[0] << 8) | obj->buffer[1]) / 1.2f ;

    return obj->value;
}
