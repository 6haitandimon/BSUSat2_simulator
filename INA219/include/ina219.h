#pragma ONCE

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
namespace INA219{
//Range configuration
#define RANGE_16V                           0 // Range 0-16 volts
#define RANGE_32V                           1 // Range 0-32 volts

//Gain configuration
#define GAIN_1_40MV                         0 // Maximum shunt voltage 40mV
#define GAIN_2_80MV                         1 // Maximum shunt voltage 80mV
#define GAIN_4_160MV                        2 // Maximum shunt voltage 160mV
#define GAIN_8_320MV                        3 // Maximum shunt voltage 320mV
// #define GAIN_AUTO                          -1 // Determine gain automatically

#define RESET_SYSTEM                         1 // bit to reset system
#define NO_RESET_SYSTEM                      0 // bit no reset system

//ADC configuration
#define ADC_9BIT                            0  // 9-bit conversion time  84us.
#define ADC_10BIT                           1  // 10-bit conversion time 148us.
#define ADC_11BIT                           2  // 11-bit conversion time 2766us.
#define ADC_12BIT                           3  // 12-bit conversion time 532us.
#define ADC_2SAMP                           9  // 2 samples at 12-bit, conversion time 1.06ms.
#define ADC_4SAMP                           10 // 4 samples at 12-bit, conversion time 2.13ms.
#define ADC_8SAMP                           11 // 8 samples at 12-bit, conversion time 4.26ms.
#define ADC_16SAMP                          12 // 16 samples at 12-bit,conversion time 8.51ms
#define ADC_32SAMP                          13 // 32 samples at 12-bit, conversion time 17.02ms.
#define ADC_64SAMP                          14 // 64 samples at 12-bit, conversion time 34.05ms.
#define ADC_128SAMP                         15 // 128 samples at 12-bit, conversion time 68.10ms.

//Addres
#define __ADDRESS_0x40                           0x40
#define __ADDRESS_0x41                           0x41
#define __ADDRESS_0x42                           0x42
#define __ADDRESS_0x43                           0x43
#define __ADDRESS_0x44                           0x44
#define __ADDRESS_0x45                           0x45
#define __ADDRESS_0x46                           0x46
#define __ADDRESS_0x47                           0x47
#define __ADDRESS_0x48                           0x48
#define __ADDRESS_0x49                           0x49

//Registrs
#define __REG_CONFIG                        0x00
#define __REG_SHUNTVOLTAGE                  0x01
#define __REG_BUSVOLTAGE                    0x02
#define __REG_POWER                         0x03
#define __REG_CURRENT                       0x04
#define __REG_CALIBRATION                   0x05

//Configuration Register 0x00
#define __RST                               15
#define __BRNG                              13
#define __PG1                               12
#define __PG0                               11
#define __BADC4                             10
#define __BADC3                             9
#define __BADC2                             8
#define __BADC1                             7
#define __SADC4                             6
#define __SADC3                             5
#define __SADC2                             4
#define __SADC1                             3
#define __MODE3                             2
#define __MODE2                             1
#define __MODE1                             0

#define __OVF                               1
#define __CNVR                              2

//Mode Settigs
#define __PWR_DOWN                          0
#define __SH_TRIG                           1
#define __BUS_TRIG                          2
#define __SH_BUS_TRIG                       3
#define __ADC_OFF                           4
#define __CONT_SH                           5
#define __CONT_BUS                          6
#define __CONT_SH_BUS                       7

//Const

#define __SHUNT_MILLIVOLTS_LSB              0.01    // 10uV
#define __BUS_MILLIVOLTS_LSB                4       // 4mV
#define __CALIBRATION_FACTOR                0.04096
#define __MAX_CALIBRATION_VALUE             0xFFFE  // Max value supported (65534 decimal)

// In the spec (p17) the current LSB factor for the minimum LSB is
// documented as 32767, but a larger value (100.1% of 32767) is used
// to guarantee that current overflow can always be detected.
#define __CURRENT_LSB_FACTOR                32770.0


/**
 * @brief Class in the controller INA219
*/

class INA219 {
    private:
    union ByByte{
    uint16_t byte;
    struct {
        uint8_t l;
        uint8_t d;
        };
    };
    i2c_inst_t *i2c;
    uint8_t SDA;
    uint8_t SCL;
    uint8_t _i2cAddr;
    float _maxExpectedAmps;
    float _battLow;
    float _battFull;
    float _currentLsb;
    float _powerLsb;
    float _shuntResistorOhms;
    ByByte _calibrationValue;
    ByByte _configuration;

    uint16_t read_register(uint8_t registr);
    void write_register(uint8_t register_address, INA219::ByByte register_value);

    public:
    /**
 * @brief initialization function for ina219
 * 
 * @param i2c - pointer to initializer structure i2c
 *              i2c0, i2c1;
 * 
 * @param SDA - SDA pin
 * 
 * @param SCL - SCL pin
 * 
 * @param addr - Sensor address ina219 on the bus i2c
 *              __ADDRESS_0x40(default), __ADDRESS_0x41 and so on.
 * 
 * @param max_expected_amps - The maximum expected current in the circuit.
                            (For proper calibration I recommend 3.2)

 * @param batt_full - Maximum battery voltage. 
                    (If not used, set the parameter to 0)

 * @param batt_low - Minimum battery voltage. 
                    (If not used, set the parameter to 0)
 
 * @param shunt_resistor_ohms - Shunt resistor resistance.

 * @param calibration_value - The exact value of the calibration register.
                            (If calibration has not been performed, set the value to 4096)
*/
    INA219(i2c_inst_t *i2c, uint8_t SDA, uint8_t SCL, uint8_t addr, float max_expected_amps, float batt_full, float batt_low, float shunt_resistor_ohms, uint16_t calibration_value);
    /**
 * @brief Configures how the INA219 will take measurements
 * 
 * @param flag_calibration - Flag that indicates the need to call 
                            the calibration function of register 0x00

 * @param RESET - Configuration register bit 15 meaning;
                RESET_SYSTEM, NO_RESET_SYSTEM

 * @param RANGE - The full scale voltage range, this is either 16V
                or 32V represented by one of the following constants;
                RANGE_16V, RANGE_32V (default).

 * @param GAIN  - The gain which controls the maximum range of the shunt
                voltage represented by one of the following constants;
                GAIN_1_40MV, GAIN_2_80MV, GAIN_4_160MV,
                GAIN_8_320MV(default).

 * @param BADC - The bus ADC resolution (9, 10, 11, or 12-bit) or
                set the number of samples used when averaging results
                represent by one of the following constants; ADC_9BIT,
                ADC_10BIT, ADC_11BIT, ADC_12BIT (default),
                ADC_2SAMP, ADC_4SAMP, ADC_8SAMP, ADC_16SAMP,
                ADC_32SAMP, ADC_64SAMP, ADC_128SAMP.

 * @param CADC - The shunt ADC resolution (9, 10, 11, or 12-bit) or
                set the number of samples used when averaging results
                represent by one of the following constants; ADC_9BIT,
                ADC_10BIT, ADC_11BIT, ADC_12BIT (default),
                ADC_2SAMP, ADC_4SAMP, ADC_8SAMP, ADC_16SAMP,
                ADC_32SAMP, ADC_64SAMP, ADC_128SAMP.

* @param MODE  - This parameter sets the operating mode of the sensor.
                __PWR_DOWN (Power-down), __SH_TRIG (Shunt voltage, triggered), 
                __BUS_TRIG (Bus voltage, triggered), __SH_BUS_TRIG (Shunt and bus, triggered),
                __ADC_OFF (ADC off (disabled)), __CONT_SH (Shunt voltage, continuous), 
                __CONT_BUS (Bus voltage, continuous), __CONT_SH_BUS (Shunt and bus, continuous) (default)
* @return Update struct ina219
*/
    void configuration(bool flag_calibration, uint8_t RESET, uint8_t RANGE, uint8_t GAIN, uint8_t BADC, uint8_t CADC, uint8_t MODE);
/**
 * @brief Function for calibrating register 0x05 according 
 * to specified parameters into the initialization function. 
 * It is recommended to use it once and adjust the values 
 * in accordance with formula (6) of the datasheet
 * 
 * @return Update struct ina219 

*/
    void calibration();
    /**
 * @brief Getting voltage in Volts
 * 
 * @return Voltage value
*/
    float get_voltage();
    /**
 * @brief Getting the current in miles Amperes
 * 
 * @param ina - Pointer to a structure with sensor parameters
 * 
 * @return Volt value (float)
*/
    float get_current_mA();
/**
 * @brief Getting the current in Amperes
 * 
 * @param ina - Pointer to a structure with sensor parameters
 * 
 * @return Current value in Amperes (float)
*/
    float get_current();
/**
 * @brief Getting the power in miles Wattah
 * 
 * @param ina - Pointer to a structure with sensor parameters
 * 
 * @return Power value in miles Wattah (float)
*/
    float get_power_mW();
/**
 * @brief Getting the power in Wattah
 * 
 * @param ina - Pointer to a structure with sensor parameters
 * 
 * @return Power value in Wattah (float)
*/
    float get_power();
/**
 * @brief Obtaining the power of the current on the shunt in miles Amperes
 * 
 * @param ina - Pointer to a structure with sensor parameters
 * 
 * @return Shunt current value in miles Amperes (float)
*/
    float get_current_from_shunt_in_mA();
/**
 * @brief Obtaining the shunt voltage in miles Volts
 * 
 * @param ina - Pointer to a structure with sensor parameters
 * 
 * @return Shunt voltage value in miles Volts (float)
*/
    float get_shunt_voltage_in_mV();
};
}