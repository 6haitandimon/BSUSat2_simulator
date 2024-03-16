#include "ina219.h"

namespace INA219{

uint16_t read_register(uint8_t addr, uint8_t register_address){
  uint8_t buf[3];
  int ret;

  ret = i2c_write_blocking(i2c0, addr, &register_address, 1, true);
  if(ret < 0)
    printf("Error write register\n");

  ret = i2c_read_blocking(i2c0, addr, buf, 2, false);
  if(ret < 0)
    printf("\nFailed to read register value\n");

  return (buf[0] << 8) | buf[1];
}

void write_register(uint8_t addr, uint8_t register_address, ByByte register_value)
{
	uint8_t write_data[3];
	write_data[0] = register_address;
	write_data[1] = register_value.d;
	write_data[2] = register_value.l;
	printf("calibration byte: %d\n", register_value.byte);
	
	i2c_write_blocking(i2c0, addr, write_data, 3, false);
}

float get_voltage(ina219 *ina){
  uint16_t value = read_register(ina->_i2c_addr, __REG_BUSVOLTAGE);

  return (float)(value >> 3) * __BUS_MILLIVOLTS_LSB / 1000.0;
}

float get_current(ina219 *ina){
	uint16_t value = read_register(ina->_i2c_addr, __REG_CURRENT);
	
	return((float)value / 1000.0);

}

float get_current_mA(ina219 *ina){
	uint16_t value = read_register(ina->_i2c_addr, __REG_CURRENT);
	
	return((float)value / 10.0);

}


float get_power_mW(ina219* ina){
	uint16_t value = read_register(ina->_i2c_addr, __REG_POWER);
	printf("power reg: %f\n, power_lsb: %f\n", (float)value, ina->_power_lsb);
	return (float)(value * ina->_power_lsb * 1000.0);
}

float get_power(ina219* ina){
	uint16_t value = read_register(ina->_i2c_addr, __REG_POWER);
	printf("power reg: %f\n, power_lsb: %f\n", (float)value, ina->_power_lsb);
	return (float)(value * ina->_power_lsb);
}

float get_current_from_shunt_in_mA(ina219 *ina){
	float value = (get_shunt_voltage_in_mV(ina) / 10);

	return (value / ina->_shunt_resistor_ohms);
}

float get_shunt_voltage_in_mV(ina219* ina){
	uint16_t value = read_register(ina->_i2c_addr, __REG_SHUNTVOLTAGE);

	if(value > 32767)
		value -= 65535;
	
	return ((float)value * 0.01);
}


ina219 _ina_init(ina219* ina, int8_t addr, float max_expected_amps, float batt_full, float batt_low, float shunt_resistor_ohms, uint16_t calibration_value){
	ina->_i2c_addr = addr;
	ina->_max_expected_amps = max_expected_amps;
  	ina->_batt_full = batt_full;
  	ina->_batt_low = batt_low;
  	ina->_shunt_resistor_ohms = shunt_resistor_ohms;
  	ina->_current_lsb = (__CALIBRATION_FACTOR / calibration_value) / shunt_resistor_ohms;
  	ina->_power_lsb = 20 * ina->_current_lsb;
  	ina->_calibration_value.byte = calibration_value;
	*ina = configuration(ina, false, NO_RESET_SYSTEM, RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, __CONT_SH_BUS);

	return *ina;
}

ina219 configuration(ina219 *ina, bool flag_calibration, uint8_t RESET, uint8_t RANGE, uint8_t GAIN, uint8_t BADC, uint8_t CADC, uint8_t MODE){
	ina->_configuration.byte = (RESET << 15) |  
							(RANGE << 13) | \
							(GAIN << 11) | \
							(BADC << 7) | \
							(CADC << 3) | \
							(MODE);

	write_register(ina->_i2c_addr, __REG_CONFIG, ina->_configuration);
	if(flag_calibration){
		*ina = calibration(ina);
	}else{
		write_register(ina->_i2c_addr, __REG_CALIBRATION, ina->_calibration_value);
	}
	return *ina;
}

ina219 calibration(ina219* ina){
	ina->_current_lsb = ina->_max_expected_amps / __CURRENT_LSB_FACTOR;
  	ina->_power_lsb = 20 * ina->_current_lsb;
  	ina->_calibration_value.byte = (uint16_t)__CALIBRATION_FACTOR / (ina->_current_lsb * ina->_shunt_resistor_ohms);

	write_register(ina->_i2c_addr, __REG_CALIBRATION, ina->_calibration_value);
	return *ina;
}
}