/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include "hardware/i2c.h"
#include "ina219.h"


#define PIN_SDA 20
#define PIN_SCL 21



int main() {
  stdio_init_all();

  i2c_init(i2c0, 115000);
  

  gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
  gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
  
  gpio_pull_up(PIN_SDA);
  gpio_pull_up(PIN_SCL);

  ina219 ina;
  ina = _ina_init(&ina, 0x40, 3.2, \
                4.2, 3.2, 0.1, 3221);

  ina = configuration(&ina, false, \
                      RESET_SYSTEM, RANGE_16V, \
                      GAIN_8_320MV, ADC_128SAMP, 
                      ADC_128SAMP, __CONT_SH_BUS);
 
  while(true){

     printf("\nMonitor voltage and current on battary\n");
     printf("Callibration register: %d, Config regist: %d \n\n", read_register(ina._i2c_addr, __REG_CALIBRATION), read_register(ina._i2c_addr, __REG_CONFIG));

    float voltage_value = get_voltage(&ina);
    float current_value = get_current_mA(&ina);
    float power_value = get_power_mW(&ina);
    float current_value_from_shunt = get_current_from_shunt(&ina);
  //  printf("current_lsb: %f\n", ina->shunt_resistor_ohms);

    

    printf("Voltage: %f V\nCurrent: %f mA\nPower: %f mW\nCurrent from shunt: %f mA\n",
    voltage_value, current_value, power_value, current_value_from_shunt);

     sleep_ms(2000);
   }  
}