#include "ina3221.h"

namespace INA3221{
uint16_t INA3221::ReadRegister(uint8_t register_address){
  uint8_t buf[3];
  buf[0] = 0;
  buf[1] = 0;
  int ret;
  ret = i2c_write_blocking(this->i2c, this->_addr, &register_address, 1, true);
  if(ret < 0)
    printf("Error write register\n");
  ret = i2c_read_blocking(this->i2c, this->_addr, buf, 2, false);
  if(ret < 0)
    printf("\nFailed to read register value\n");
  return (buf[0] << 8) | buf[1];
}

void INA3221::WriteRegister(uint8_t register_address, INA3221::ByByte register_value)
{
	uint8_t write_data[3];
	write_data[0] = register_address;
	write_data[1] = register_value.d;
	write_data[2] = register_value.l;
	printf("calibration byte: %d\n", register_value.byte);
	
	i2c_write_blocking(this->i2c, this->_addr, write_data, 3, false);
}

INA3221::INA3221(i2c_inst_t *i2c, uint8_t SDA, uint8_t SCL, uint8_t addr, float batt_full, float batt_low, float shunt_resistor_ohms){
	
	i2c_init(i2c, 400000);
	gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SCL);
    gpio_pull_up(SDA);

	this->i2c = i2c;
	this->SDA = SDA;
	this->SCL = SCL;
	this->_addr = addr;
	this->_batt_full = batt_full;
	this->_batt_low = batt_low;
	this->_shunt_resistor_ohms = shunt_resistor_ohms;
}


float INA3221::GetVoltage(uint8_t channel){
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
  	uint16_t value = ReadRegister(__REG_BUSVOLTAGE);

  	return (float)(value >> 3) * 0.008;
}

float INA3221::GetCurrent(uint8_t channel){
	return (float)GetShuntVoltage(channel) / this->_shunt_resistor_ohms;

}

void INA3221::Configuration(){

	ByByte configuration;
	configuration.byte = (RESET_SYSTEM << 15) |  
						(ALL_CHANNEL << 12) | \
                        (AVG2_3 << 9) | \
                        (Vbus_CTV2_4 << 6) | \
                        (Vsh_CTV2_4 << 3) | \
                        (__MODE7);


	WriteRegister(__REG_CONFIG, configuration);
}

float INA3221::GetShuntVoltage(uint8_t channel){
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
	uint16_t value = ReadRegister(__REG_SHUNTVOLTAGE);

	if(value > 32767)
		value -= 65535;
	
	return (float)(value >> 3) * 0.00004;
}

float INA3221::GetPower(uint8_t channel){
	return (float)(GetCurrent(channel) * GetVoltage(channel));
}
}