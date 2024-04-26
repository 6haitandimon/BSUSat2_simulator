#include "ina219.h"

namespace INA219 {
    uint16_t INA219::read_register(uint8_t register_address) {
        uint8_t buf[3];
        int ret;

        ret = i2c_write_blocking(i2c0, this->_i2cAddr, &register_address, 1, true);
        if (ret < 0)
            printf("Error write register\n");

        ret = i2c_read_blocking(i2c0, this->_i2cAddr, buf, 2, false);
        if (ret < 0)
            printf("\nFailed to read register value\n");

        return (buf[0] << 8) | buf[1];
    }

    void INA219::write_register(uint8_t register_address, INA219::ByByte register_value) {
        uint8_t write_data[3];
        write_data[0] = register_address;
        write_data[1] = register_value.d;
        write_data[2] = register_value.l;
        printf("calibration byte: %d\n", register_value.byte);

        i2c_write_blocking(i2c0, this->_i2cAddr, write_data, 3, false);
    }

    float INA219::get_voltage() {
        uint16_t value = read_register(__REG_BUSVOLTAGE);

        return (float) (value >> 3) * __BUS_MILLIVOLTS_LSB / 1000.0;
    }

    float INA219::get_current() {
        uint16_t value = read_register(__REG_CURRENT);

        return ((float) value / 1000.0);

    }

    float INA219::get_current_mA() {
        uint16_t value = read_register(__REG_CURRENT);

        return ((float) value / 10.0);

    }


    float INA219::get_power_mW() {
        uint16_t value = read_register(__REG_POWER);
        printf("power reg: %f\n, power_lsb: %f\n", (float) value, this->_powerLsb);
        return (float) (value * this->_powerLsb * 1000.0);
    }

    float INA219::get_power() {
        uint16_t value = read_register(__REG_POWER);
        printf("power reg: %f\n, power_lsb: %f\n", (float) value, this->_powerLsb);
        return (float) (value * this->_powerLsb);
    }

    float INA219::get_current_from_shunt_in_mA() {
        float value = (get_shunt_voltage_in_mV() / 10);
        return (value / this->_shuntResistorOhms);
    }

    float INA219::get_shunt_voltage_in_mV() {
        uint16_t value = read_register(__REG_SHUNTVOLTAGE);

        if (value > 32767)
            value -= 65535;

        return ((float) value * 0.01);
    }


    INA219::INA219(i2c_inst_t *i2c, uint8_t SDA, uint8_t SCL, uint8_t addr, float max_expected_amps, float batt_full,
                   float batt_low, float shunt_resistor_ohms, uint16_t calibration_value) {
        i2c_init(i2c, 400000);
        gpio_set_function(SDA, GPIO_FUNC_I2C);
        gpio_set_function(SCL, GPIO_FUNC_I2C);
        gpio_pull_up(SCL);
        gpio_pull_up(SDA);
        this->i2c = i2c;
        this->SDA = SDA;
        this->SCL = SCL;
        this->_i2cAddr = addr;
        this->_maxExpectedAmps = max_expected_amps;
        this->_battFull = batt_full;
        this->_battLow = batt_low;
        this->_shuntResistorOhms = shunt_resistor_ohms;
        this->_currentLsb = (__CALIBRATION_FACTOR / calibration_value) / shunt_resistor_ohms;
        this->_powerLsb = 20 * this->_currentLsb;
        this->_calibrationValue.byte = calibration_value;
        for(int i = addr; i < addr + 9; i++){
            this->_i2cAddr = addr;
            configuration(false, RESET_SYSTEM, RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, __CONT_SH_BUS);
        }
    }

    void
    INA219::configuration(bool flag_calibration, uint8_t RESET, uint8_t RANGE, uint8_t GAIN, uint8_t BADC, uint8_t CADC,
                          uint8_t MODE) {
        this->_configuration.byte = (RESET << 15) |
                                    (RANGE << 13) | \
                            (GAIN << 11) | \
                            (BADC << 7) | \
                            (CADC << 3) | \
                            (MODE);

        write_register(__REG_CONFIG, this->_configuration);
        if (flag_calibration) {
            calibration();
        } else {
            write_register(__REG_CALIBRATION, this->_calibrationValue);
        }
    }

    void INA219::calibration() {
        this->_currentLsb = this->_maxExpectedAmps / __CURRENT_LSB_FACTOR;
        this->_powerLsb = 20 * this->_currentLsb;
        this->_calibrationValue.byte = (uint16_t) __CALIBRATION_FACTOR / (this->_currentLsb * this->_shuntResistorOhms);

        write_register(__REG_CALIBRATION, this->_calibrationValue);

    }

    void INA219::changeAddres(uint8_t addr){
        this->_i2cAddr = addr;
        return;
    }
}