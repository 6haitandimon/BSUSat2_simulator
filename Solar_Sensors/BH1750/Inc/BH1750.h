/**
**************************************************************************
* @file           : BH1750.h
* @brief           Драйвер для работы с датчиком интенсивности света BH1750.
* @author         : steeng
**************************************************************************
*/
#ifndef BH1750_H
#define BH1750_H

#include "stdbool.h"
#include "bh1750_stm32_port.h" // Файл порта
/**
 *
 * @brief Адрес датчика на шине i2c в зависимости от подключения выхода ADDR \n Адрес уже сдвинутый на 1 бит
 */
enum DeviceAddress{
    // без земли
    NO_GND_ADDR_WR = 0xB8,
    NO_GND_ADDR_RD = 0xB9,
    //с землей
    GND_ADDR_WR = 0x46,
    GND_ADDR_RD = 0x47
};
/**
 *
 * @brief команды для общения с датчиком
 */
enum Commands{
    POWER_DOWN = 0x00,
    POWER_ON = 0x01,
    RESET_ = 0x03, // Сбросить значение регистра данных. Команда сброса неприемлема в режиме отключения питания.
    H_RES_MODE = 0x10, // Время измерения 120 ms. Разрешение 0.5 lx
    H_RES_MODE2 = 0x11, // Время измерения 120 ms. Разрешение 1.0 lx
    L_RES_MODE = 0x13, // Время измерения 16 ms. Разрешение 4.0 lx
    ONE_H_RES_MODE = 0x20, // Начать измерение с разрешением 1 lx. Время измерения обычно составляет 120 ms. После измерения автоматически устанавливается режим отключения питания.
    ONE_H_RES_MODE2 = 0x21, // Начать измерение с разрешением 0,5 lx. Время измерения обычно составляет 120 ms. После измерения автоматически устанавливается режим отключения питания.
    ONE_L_RES_MODE = 0x23, // Начать измерение с разрешением 4 lx. Время измерения обычно составляет 16 ms. После измерения автоматически устанавливается режим отключения питания.
    CHANGE_TIME_HIGH = 0x30, // Изменить время измерения.
    CHANGE_TIME_LOW = 0x60 // Изменить время измерения.
};

/**
 *
 * @struct BH1750
 * @brief Структура, представляющая сенсор
 * @param I2C_HandleTypeDef Handler I2C для связи с сенсором на шине
 * @param address_RD Адрес чтения с датчика.
 * @param address_WR Адрес записи команды в датчик.
 * @param value Текущее значение, считанное с сенсора
 * @param buffer Буфер для хранения данных при общении с сенсором
 */
typedef struct BH1750 {
    sw_i2c_t * i2c_handle;
    StatusTypeDef status;
    uint8_t address_RD;
    uint8_t address_WR;
    uint8_t buffer[2];
    float value;
}BH1750_t;

BH1750_t BH1750_Init(sw_i2cT* i2c_handle,bool ADDR_GND);
StatusTypeDef ChechStatus(const BH1750_t * obj);
StatusTypeDef BH1750_ON(const BH1750_t * obj);
StatusTypeDef BH1750_WR(const BH1750_t * obj,uint8_t *cmd);
StatusTypeDef BH1750_RD(BH1750_t * obj);
float BH1750_get_value(BH1750_t *obj);


#endif //BH1750_H
