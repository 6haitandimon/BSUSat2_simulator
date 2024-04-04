from machine import I2C, Pin
import time

INA3221_ADDRESS_1 = 0x40
INA3221_ADDRESS_2 = 0x41
INA3221_REG_SHUNTVOLTAGE_1 = 0x01
INA3221_REG_BUSVOLTAGE_1 = 0x02
INA3221_REG_SHUNTVOLTAGE_2 = 0x03
INA3221_REG_BUSVOLTAGE_2 = 0x04
INA3221_REG_SHUNTVOLTAGE_3 = 0x05
INA3221_REG_BUSVOLTAGE_3 = 0x06
INA3221_SHUNT_RESISTANCE = 0.2

i2c = I2C(0, scl=Pin(21), sda=Pin(20), freq=400000)


# CONFIG_REGISTER = 0x00
# default conditions
# i2c.writeto_mem(INA3221_ADDRESS_1, CONFIG_REGISTER, bytes([0b00001111]))
# i2c.writeto_mem(INA3221_ADDRESS_2, CONFIG_REGISTER, bytes([0b00001111]))


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

        voltage_ix_minus = read_voltage_bus(INA3221_ADDRESS_1, 3)
        voltage_ix_plus = read_voltage_bus(INA3221_ADDRESS_2, 3)
        voltage_iy_minus = read_voltage_bus(INA3221_ADDRESS_1, 1)
        voltage_iy_plus = read_voltage_bus(INA3221_ADDRESS_2, 1)

        current_ix_minus = (-1) * read_current(INA3221_ADDRESS_1, 3)  # (-1)* - compensation schema-specific coeff
        current_ix_plus = (-1) * read_current(INA3221_ADDRESS_2, 3)
        current_iy_minus = (-1) * read_current(INA3221_ADDRESS_1, 1)
        current_iy_plus = (-1) * read_current(INA3221_ADDRESS_2, 1)

        if count % 10 == 0:
            print(f"V_X_minus  I_Y_minus    V_X_plus  I_X_plus    V_Y_minus  I_Y_minus   V_Y_plus  I_Y_plus")
        print("{:7.4f}".format(voltage_ix_minus), "   {:8.5f}".format(current_ix_minus),
              "    {:7.4f}".format(voltage_ix_plus), "  {:8.5f}".format(current_ix_plus),
              "   {:7.4f}".format(voltage_iy_minus), "   {:8.5f}".format(current_iy_minus),
              "   {:7.4f}".format(voltage_iy_plus), "  {:8.5f}".format(current_iy_plus))
        count = count + 1
        # print(f"{voltage_ix_minus}\t\t{current_ix_minus}\t\t{voltage_ix_plus}\t\t{current_ix_plus}\t\t{voltage_iy_minus}\t\t{current_iy_minus}\t\t{voltage_iy_plus}\t\t{current_iy_plus}")
        time.sleep(0.5)

except KeyboardInterrupt:
    pass
