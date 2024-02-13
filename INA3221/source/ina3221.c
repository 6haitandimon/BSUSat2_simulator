#include "ina3221.h"



bool reservedAdress(uint8_t addr){
  return(addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

uint16_t read_register(uint8_t addr, uint8_t register_address){
  uint8_t buf[3];
  buf[0] = 0;
  buf[1] = 0;
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

float get_voltage(ina3221 *ina, uint8_t channel){
	uint16_t __REG_BUSVOLTAGE = 0;
	if(channel == 1)
		__REG_BUSVOLTAGE = __REG_BUSVOLTAGE_1;
	else if(channel == 2)
		__REG_BUSVOLTAGE = __REG_BUSVOLTAGE_2;
	else if(channel == 3)
		__REG_BUSVOLTAGE = __REG_BUSVOLTAGE_3;
	else{
		printf("Error channel");
		__REG_BUSVOLTAGE = 0;
	}
  	uint16_t value = read_register(ina->_i2c_addr, __REG_BUSVOLTAGE);

  	return (float)(value >> 3) * 0.008;
}

float get_current(ina3221 *ina, uint8_t channel){
	return (float)get_shunt_voltage(ina, channel) / ina->_shunt_resistor_ohms;

}

ina3221 configuration(ina3221 *ina){
	// ina->_current_lsb = ina->_max_expected_amps / 32768;
	//ceil((ina->_max_expected_amps / 32768) * 1e+4) / 1e+4
	// ina->_power_lsb = 20 * ina->_current_lsb;
	// ina->_calibration_value.byte = (0.04096 / (ina->_current_lsb * ina->_shunt_resistor_ohms));
	// ina->_calibration_value.byte = (ina->_calibration_value.byte << 1);

	ByByte configuration;
	configuration.byte = (RESET_SYSTEM << 15) |  
						(ALL_CHANNEL << 12) | \
                        (AVG2_3 << 9) | \
                        (Vbus_CTV2_4 << 6) | \
                        (Vsh_CTV2_4 << 3) | \
                        (__MODE7);
	
	// printf("callibration value: %d\n", ina->_calibration_value.byte);


	write_register(ina->_i2c_addr, __REG_CONFIG, configuration);
	// write_register(ina->_i2c_addr, __REG_CALIBRATION, ina->_calibration_value);
	
	return *ina;
}

// float get_current_from_shunt(ina3221 *ina){
// 	float value = (get_shunt_voltage_in_mV(ina) / 10);

// 	return (value / ina->_shunt_resistor_ohms);
// }

float get_shunt_voltage(ina3221* ina, uint8_t channel){
	uint16_t __REG_SHUNTVOLTAGE = 0;
	if(channel == 1)
		__REG_SHUNTVOLTAGE = __REG_BUSVOLTAGE_1;
	else if(channel == 2)
		__REG_SHUNTVOLTAGE = __REG_BUSVOLTAGE_2;
	else if(channel == 3)
		__REG_SHUNTVOLTAGE = __REG_BUSVOLTAGE_3;
	else{
		printf("Error channel");
		__REG_SHUNTVOLTAGE = 0;
	}
	uint16_t value = read_register(ina->_i2c_addr, __REG_SHUNTVOLTAGE);

	if(value > 32767)
		value -= 65535;
	
	return (float)(value >> 3) * 0.00004;
}

float get_power(ina3221* ina, uint8_t channel){
	return (float)(get_current(ina, channel) * get_voltage(ina, channel));
}

// float calculate_power(ina3221* ina){
// 	float value = 

// 	return(value / 1000000.0);
// }

ina3221 _ina_init(ina3221* ina, int8_t addr, /*float max_expected_amps,*/ float batt_full, float batt_low, float shunt_resistor_ohms){
	ina->_i2c_addr = addr;
	// ina->_max_expected_amps = max_expected_amps;
  	ina->_batt_full = batt_full;
  	ina->_batt_low = batt_low;
  	ina->_shunt_resistor_ohms = shunt_resistor_ohms;
  	// ina->_power_lsb = 0;
  	// ina->_current_lsb = 0;
  	// ina->_calibration_value.byte = 0;

	return *ina;
}


// void calibrate(int bus_volts_max, float shunt_volts_max, float max_expected_amps, uint8_t addr)
// {
// 	float max_possible_amps = shunt_volts_max / _shunt_ohms;
// 	_current_lsb = determine_current_lsb(max_expected_amps, max_possible_amps);
// 	_power_lsb = _current_lsb * 20.0;
// 	uint16_t calibration = (uint16_t) trunc(__CALIBRATION_FACTOR / (_current_lsb * _shunt_ohms));
// 	write_register(addr, __REG_CALIBRATION, calibration);
// }

// float determine_current_lsb(float max_expected_amps, float max_possible_amps)
// {
// 	float current_lsb;

// 	float nearest = roundf(max_possible_amps * 1000.0) / 1000.0;
// 	if (max_expected_amps > nearest) {
// 		char buffer[65];
// 		sprintf(buffer, "Expected current %f A is greater than max possible current %f A", max_expected_amps, max_possible_amps);
// 		perror(buffer);
// 	}

// 	if (max_expected_amps < max_possible_amps) {
// 		current_lsb = max_expected_amps / __CURRENT_LSB_FACTOR;
// 	} else {
// 		current_lsb = max_possible_amps / __CURRENT_LSB_FACTOR;
// 	}
	
// 	if (current_lsb < _min_device_current_lsb) {
// 		current_lsb = _min_device_current_lsb;
// 	}
// 	return current_lsb;
// }