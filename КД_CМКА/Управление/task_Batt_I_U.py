from machine import I2C, Pin
import time
import math

INA3221_ADDRESS_1 = 0x42
INA3221_ADDRESS_2 = 0x43
INA3221_REG_SHUNTVOLTAGE_1 = 0x01
INA3221_REG_BUSVOLTAGE_1 = 0x02
INA3221_REG_SHUNTVOLTAGE_2 = 0x03
INA3221_REG_BUSVOLTAGE_2 = 0x04
INA3221_REG_SHUNTVOLTAGE_3 = 0x05
INA3221_REG_BUSVOLTAGE_3 = 0x06
INA3221_SHUNT_RESISTANCE = 0.2
INA3221_SHUNT_T_RESISTANCE = 270

i2c = I2C(0, scl=Pin(21), sda=Pin(20), freq=400000)


# CONFIG_REGISTER = 0x00
# default conditions
# i2c.writeto_mem(INA3221_ADDRESS_SLOT_1, CONFIG_REGISTER, bytes([0b00001111]))
# i2c.writeto_mem(INA3221_ADDRESS_SLOT_2, CONFIG_REGISTER, bytes([0b00001111]))


def read_voltage_bus(ina_address, channel):
    reg = 0
    if channel == 1:
        reg = INA3221_REG_BUSVOLTAGE_1
    elif channel == 2:
        reg = INA3221_REG_BUSVOLTAGE_2
    elif channel == 3:
        reg = INA3221_REG_BUSVOLTAGE_3
    else:
        return 0.0

    data = i2c.readfrom_mem(ina_address, reg, 2)
    raw_voltage = (data[0] << 8 | data[1])  # MSB transmitted fist
    voltage = (raw_voltage >> 3) * 0.008
    return voltage


def read_voltage_shunt(ina_address, channel):
    reg = 0
    if channel == 1:
        reg = INA3221_REG_SHUNTVOLTAGE_1
    elif channel == 2:
        reg = INA3221_REG_SHUNTVOLTAGE_2
    elif channel == 3:
        reg = INA3221_REG_SHUNTVOLTAGE_3
    else:
        return 0.0

    data = i2c.readfrom_mem(ina_address, reg, 2)
    raw_voltage = (data[0] << 8 | data[1])  # MSB transmitted fist
    if raw_voltage > 32767:
        raw_voltage -= 65536
    voltage = (raw_voltage >> 3) * 0.00004
    return voltage


def read_current(ina_address, channel):
    return read_voltage_shunt(ina_address, channel) / INA3221_SHUNT_RESISTANCE


try:
    count = 0
    while True:

        voltage_bat1_1 = read_voltage_bus(INA3221_ADDRESS_1, 1)
        current_bat1_1 = read_current(INA3221_ADDRESS_1, 1)

        Tvoltage_bat_1 = read_voltage_bus(INA3221_ADDRESS_1, 2)
        TvoltageShunt1 = read_voltage_shunt(INA3221_ADDRESS_1, 2)

        R_termistor1 = Tvoltage_bat_1 / (TvoltageShunt1 / INA3221_SHUNT_T_RESISTANCE);
        A = 0.8781625571e-3
        B = 2.531972392e-4
        C = 1.840753501e-7
        Temperature1 = 1 / (
                    A + B * math.log(R_termistor1) + C * math.log(R_termistor1) * math.log(R_termistor1) * math.log(
                R_termistor1))
        Temperature1 -= 273.15

        voltage_bat2_1 = read_voltage_bus(INA3221_ADDRESS_1, 3)
        current_bat2_1 = read_current(INA3221_ADDRESS_1, 3)

        voltage_bat1_2 = read_voltage_bus(INA3221_ADDRESS_2, 1)
        current_bat1_2 = read_current(INA3221_ADDRESS_2, 1)

        Tvoltage_bat_2 = read_voltage_bus(INA3221_ADDRESS_2, 2)
        TvoltageShunt2 = read_voltage_shunt(INA3221_ADDRESS_2, 2)

        R_termistor2 = Tvoltage_bat_2 / (TvoltageShunt2 / INA3221_SHUNT_T_RESISTANCE);
        Temperature2 = 1 / (
                    A + B * math.log(R_termistor2) + C * math.log(R_termistor2) * math.log(R_termistor2) * math.log(
                R_termistor2))
        Temperature2 -= 273.15

        voltage_bat2_2 = read_voltage_bus(INA3221_ADDRESS_2, 3)
        current_bat2_2 = read_current(INA3221_ADDRESS_2, 3)

        if count % 10 == 0:
            print(
                f"Battary1:Voltage11  Current11  Voltage21  Current21  Temperature1 Battary2:Voltage12  Current12  Voltage22  Current22  Temperature2")

        print("        {:7.4f}".format(voltage_bat1_1), "    {:8.5f}".format(current_bat1_1),
              "   {:7.4f}".format(voltage_bat2_1), "  {:8.5f}".format(current_bat2_1), "  {:7.3f}".format(Temperature1),
              "             {:7.4f}".format(voltage_bat1_2), "    {:8.5f}".format(current_bat1_2),
              "   {:7.4f}".format(voltage_bat2_2), "  {:8.5f}".format(current_bat2_2), "  {:7.3f}".format(Temperature2))
        count = count + 1

        # print(f"{voltage_bat1_1}\t{current_bat1_1}\t{voltage_bat2_1}\t{current_bat2_1}\t{Temperature1}\t{voltage_bat1_2}\t{current_bat1_2}\t{voltage_bat2_2}\t{current_bat2_2}\t{Temperature2}") #output string for tunnel via COM-port

        time.sleep(0.5)

except KeyboardInterrupt:
    pass
