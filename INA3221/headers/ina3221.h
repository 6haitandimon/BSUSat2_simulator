#define RESET_SYSTEM                        1 // bit to reset system

//Averaging mode. 
//These bits set the number of samples that are collected and averaged together.

//channel ON
#define CHANNEL_1                           4 // only first channel
#define CHANNEL_2                           2 // only second channel
#define CHANNEL_3                           1 // only third channel
#define ALL_CHANNEL                         7 // all channel


#define AVG2_0                              0  // 1. (default)
#define AVG2_1                              1  // 4.
#define AVG2_2                              2  // 16.
#define AVG2_3                              3  // 64.
#define AVG2_4                              4  // 128.
#define AVG2_5                              5  // 256.
#define AVG2_6                              6  // 512.
#define AVG2_7                              7  // 1024.

//Bus-voltage conversion time. 
//These bits set the conversion time for the bus-voltage measurement.

#define Vbus_CTV2_0                         0 //140us.
#define Vbus_CTV2_1                         1 //240us.
#define Vbus_CTV2_2                         2 //332us.
#define Vbus_CTV2_3                         3 //588us.
#define Vbus_CTV2_4                         4 //1.1ms.(default)
#define Vbus_CTV2_5                         5 //2.116ms.
#define Vbus_CTV2_6                         6 //4.156ms.
#define Vbus_CTV2_7                         7 //8.244ms.

//Shunt-voltage conversion time. These bits set the conversion time for the shunt-voltage measurement.
//The conversion-time bit settings for VSHCT2-0 are the same as VBUSCT2-0 (bits 8-6) listed in the previous row.

#define Vsh_CTV2_0                         0 //140us.
#define Vsh_CTV2_1                         1 //240us.
#define Vsh_CTV2_2                         2 //332us.
#define Vsh_CTV2_3                         3 //588us.
#define Vsh_CTV2_4                         4 //1.1ms.(default)
#define Vsh_CTV2_5                         5 //2.116ms.
#define Vsh_CTV2_6                         6 //4.156ms.
#define Vsh_CTV2_7                         7 //8.244ms.

//registrs

#define __REG_CONFIG                        0x00
#define __REG_SHUNTVOLTAGE_1                0x01
#define __REG_BUSVOLTAGE_1                  0x02
#define __REG_SHUNTVOLTAGE_2                0x03
#define __REG_BUSVOLTAGE_2                  0x04
#define __REG_SHUNTVOLTAGE_3                0x05
#define __REG_BUSVOLTAGE_3                  0x06

#define __MODE7                             7 //Shunt and bus, continuous (default)
#define __MODE6                             6 //Bus voltage, continuous
#define __MODE5                             5 //Shunt voltage, continuous
#define __MODE4                             4 //Power-down
#define __MODE3                             3 //Shunt and bus, single-shot (triggered)
#define __MODE2                             2 //Bus voltage, single-shot (triggered)
#define __MODE1                             1 //Shunt voltage, single-shot (triggered)
#define __MODE0                             0 //Power-down



#include "hardware/i2c.h"
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

typedef union ByByte{
    uint16_t byte;
    struct {
        uint8_t l;
        uint8_t d;
    };
} ByByte;

typedef struct ina3221
{
    int8_t _i2c_addr;
    float _batt_low;
    float _batt_full;
    float _shunt_resistor_ohms;
    // ByByte _calibration_value;
    
} ina3221;



bool reservedAdress(uint8_t);

uint16_t read_register(uint8_t, uint8_t);
void write_register(uint8_t, uint8_t, ByByte);

ina3221 configuration(ina3221 *ina);
ina3221 _ina_init(ina3221* ina, int8_t addr, float batt_full, float batt_low, float shunt_resistor_ohms);

float get_voltage(ina3221 *ina, uint8_t channel);
float get_current(ina3221 *ina, uint8_t channel);
float get_power(ina3221 *ina, uint8_t channel);
// float get_current_from_shunt(ina3221*);
float get_shunt_voltage(ina3221* ina, uint8_t channel);


