from machine import I2C, Pin
from math import trunc
import time

INA219_REG_CONFIG     = 0x00
INA219_REG_VOLT_SHUNT = 0x01
INA219_REG_VOLT_BUS   = 0x02
INA219_REG_POWER      = 0x03
INA219_REG_CURRENT    = 0x04
INA219_REG_CALIB      = 0x05

INA219_ADDR_SLOT     = 0x44

INA219_SHUNT_RESISTANCE = 0.1
INA219_CURRENT_LSB = 0.320 / 32768

PCA9554_ADDRESS_1 = 0x3F #601/1
PCA9554_ADDRESS_2 = 0x38 #601/2

PCA9554_REG_INPUT_PORT = 0x00
PCA9554_REG_OUTPUT_PORT = 0x01
PCA9554_REG_CONFIG = 0x03

CONFIG_PCA9554_1 = b'\x49' #0b 0100 1001 set I/O pin
CONFIG_PCA9554_2 = b'\x70' #0b 0111 0000 set I/O pin

i2c0 = machine.I2C(0, scl=Pin(21), sda=Pin(20), freq=400000)
i2c1 = machine.I2C(1, scl=Pin(7), sda=Pin(6), freq=400000)

i2c0.writeto_mem(PCA9554_ADDRESS_1, PCA9554_REG_CONFIG, CONFIG_PCA9554_1)
i2c0.writeto_mem(PCA9554_ADDRESS_2, PCA9554_REG_CONFIG, CONFIG_PCA9554_2)

analog_value1 = machine.ADC(28) #Vbat
analog_value2 = machine.ADC(27) #Vusb 
analog_value3 = machine.ADC(26) #VsolarBus
analog_value4 = machine.ADC(4)  #Temperature 
#FLG_PIN = Pin(16, Pin.IN)   


def short_to_bytes(val):
    return bytearray([(val >> 8) & 0xFF, val & 0xFF])

def slot_controle(slot_number,operation:str):
   
     if slot_number == 1:
         slot_bit = 0x02
         pca_addr = 0x3F
     elif slot_number == 2:
         slot_bit = 0x04
         pca_addr = 0x3F
     elif slot_number == 3:  #601|2
         slot_bit = 0x01
         pca_addr = 0x38
     elif slot_number == 4:  #601|2
         slot_bit = 0x02
         pca_addr = 0x38
     elif slot_number == 5:  #601|2
         slot_bit = 0x04
         pca_addr = 0x38
     elif slot_number == 6:  #601|2
         slot_bit = 0x08
         pca_addr = 0x38
     elif slot_number == 7:  #601|2
         slot_bit = 0x80
         pca_addr = 0x38
     elif slot_number == 8:
         slot_bit = 0x20
         pca_addr = 0x3F
     elif slot_number == 9:
         slot_bit = 0x80
         pca_addr = 0x3F
     elif slot_number == 10:
        slot_bit = 0x10
        pca_addr = 0x3F
     else:
        print ("Invalid slot number.")
        return
    
     if operation == "enable":
         i2c0.writeto_mem(pca_addr, PCA9554_REG_OUTPUT_PORT, slot_bit.to_bytes(2, 'big'))
     else:
         config_reg_status = int.from_bytes(i2c0.readfrom_mem(pca_addr, PCA9554_REG_INPUT_PORT, 1), 'big')
         print(config_reg_status) #for debuq
         slot_bit = config_reg_status ^ slot_bit
         print(slot_bit) #for debuq
         i2c0.writeto_mem(pca_addr, PCA9554_REG_OUTPUT_PORT, slot_bit.to_bytes(2, 'big'))

def disable_all_slots():
    i2c0.writeto_mem(PCA9554_ADDRESS_1, PCA9554_REG_OUTPUT_PORT, bytes([0]))
    i2c0.writeto_mem(PCA9554_ADDRESS_2, PCA9554_REG_OUTPUT_PORT, bytes([0]))


def read_overcurrent_status(PCA9554_ADDRESS):
     
    overcurrent_status = int.from_bytes(i2c0.readfrom_mem(PCA9554_ADDRESS, PCA9554_REG_INPUT_PORT, 1), 'big')
    slot_overcurrent = "none"
    if PCA9554_ADDRESS == PCA9554_ADDRESS_1: 
        if not(overcurrent_status & 0x01):
            slot_overcurrent = "System power "
        if not(overcurrent_status & 0x08):
            slot_overcurrent += "Slot 1,2 "
        if not (overcurrent_status & 0x40):
            slot_overcurrent += "Slot 9,10 "    
    elif  PCA9554_ADDRESS == PCA9554_ADDRESS_2:
        if not(overcurrent_status & 0x10):
            slot_overcurrent += "Slot 5,6 "
        if not(overcurrent_status & 0x20):
            slot_overcurrent += "Slot 7,8 "
        if not(overcurrent_status & 0x40):
            slot_overcurrent += "Slot 3,4"    
    return slot_overcurrent, overcurrent_status       
   

def read_current_voltage(slot_number):
    addr = INA219_ADDR_SLOT + slot_number - 1
    #i2c0.writeto_mem(addr, INA219_REG_CONFIG, bytes([0b00000011]))
    
    calib_value = trunc(0.04096 / ( INA219_SHUNT_RESISTANCE * INA219_CURRENT_LSB));
    #print(f"calib {calib_value}")
    i2c0.writeto_mem(addr, INA219_REG_CALIB, short_to_bytes(calib_value))
    time.sleep(1)
    
    current = int.from_bytes(i2c0.readfrom_mem(addr, INA219_REG_CURRENT, 2), 'big') * INA219_CURRENT_LSB
     
    shunt_voltage = int.from_bytes(i2c0.readfrom_mem(addr, INA219_REG_VOLT_SHUNT, 2), 'big')
    
    if shunt_voltage > 32767:
        shunt_voltage -= 65536
    shunt_voltage = shunt_voltage * 0.00001
    bus_voltage   = (int.from_bytes(i2c0.readfrom_mem(addr, INA219_REG_VOLT_BUS, 2)  , 'big', True) >> 3) * 0.004
    
    return shunt_voltage, bus_voltage, current

#Slots
#1-Radio1
#2-Battery1 state ON after Rbf remove
#3-Reserve
#4-Camera
#5-GPS
#6-Reserve
#7-Battery2 state ON after Rbf remove
#8-Coils
#9-Radio2
#10-SolarSensors

#slot 2 and 7 always оn after Rbf remove
try:
    disable_all_slots()
    
     #scan devices into service-I2C0 bus    
    devices0 = i2c0.scan() 
    if len(devices0) == 0:
     print("No i2c0 device !")
    else:
     print("i2c0 devices found:",len(devices0))
    for dev in devices0:
        print(f"{dev:X}",end =" ")
    print()
        
    #scan devices into adding-I2C1 bus
    #this check cost after released firmware for modules microcontrollers  
    #devices1 = i2c1.scan() 
    #if len(devices1) == 0:
    # print("No i2c1 device !")
    #else:
    # print("i2c1 devices found:",len(devices1))
    #for dev in devices1:
    #    print(f"{dev:X}",end =" ")
    #print()
    
    #convertion_factor = 3.009/(65536)*(5/3)
    convertion_factor = 3.009/(4095)*(5/3)      
    reading = (analog_value1.read_u16()>>4)*convertion_factor
    print("Vbat: ",reading)
          
    reading = (analog_value2.read_u16()>>4)*convertion_factor
    print("Vusb: ",reading)
            
    reading = (analog_value3.read_u16()>>4)*convertion_factor
    print("VsolarBus: ",reading)
    
    #reading = analog_value4.read_u16()*3.009/65535    #16 bit code from pico
    reading = (analog_value4.read_u16()>>4)*3.009/4095 #real 12bit ADC
    print(reading)
    temperature = 27 - (reading - 0.706)/(0.001721)    # rp2040 chips temperature
        
    print("Temperature_chips: ",temperature) 
       
    
    #while True:
    for i in range(0,1):
        time.sleep(0.1) 
        #regular cicle 
        
        #reading System Power U,I before enable slot  
                 
        voltage_s, voltage_b, current = read_current_voltage(0xB)
        #print(f"System Power info: Voltage Shunt = {voltage_s} V, Voltage Bus = {voltage_b} V, Current = {current}")
        print("Voltage Shunt ={:7.4f}".format(voltage_s),"Voltage Bus ={:7.4f}".format(voltage_b),"Current ={:8.5f}".format(current))
        #print(f"{voltage_s}\t{voltage_b}\t{current}")
        time.sleep(0.1)        
        
        slot = int(input("Enter slots number = ")) #command from upper level programm
            
        for slot in range(slot,slot+1):  #1-Radio1 #2-Battery1 #3-Reserve #4-Camera #5-GPS #6-Reserve #7-Battery2 #8-Coils #9-Radio2 #10-SolarSensors
           
            slot_controle(slot,"enable")
            time.sleep(0.1)
               
        for i in range(0,3):  #repeat read parameters  
            voltage_s, voltage_b, current = read_current_voltage(slot)
            #print(f"Slot {slot}: Voltage Shunt = {voltage_s} V, Voltage Bus = {voltage_b} V, Current = {current}") #output string for regular debuq
            print("Slot {:d}".format(slot),"Voltage Shunt ={:7.4f}".format(voltage_s),"Voltage Bus ={:7.4f}".format(voltage_b),"Current ={:8.5f}".format(current))
            #print(f"{voltage_s}\t{voltage_b}\t{current}") #output string for tunnel via COM-port
            time.sleep(0.1)          
                        
        #reading System Power U,I after enable slot          
           
        for i in range(0,3):  
            voltage_s, voltage_b, current = read_current_voltage(0xB)
            print(f"System Power info: Voltage Shunt = {voltage_s} V, Voltage Bus = {voltage_b} V, Current = {current}")
            #print(f"{voltage_s}\t{voltage_b}\t{current}") #output string for tunnel via COM-port
            time.sleep(0.1)              
        
        slot_overcurrent, overcurrent_status  = read_overcurrent_status(PCA9554_ADDRESS_1)       
        #print(f"slot_1-2-9-10-11_overcurrent = {slot_overcurrent}, overcurrent_status = {overcurrent_status}")
        print(f"slot_1-2-9-10-11_overcurrent = {slot_overcurrent}")
        slot_overcurrent, overcurrent_status  = read_overcurrent_status(PCA9554_ADDRESS_2)
        #print(f"slot_3-4-5-6-7_8_overcurrent = {slot_overcurrent}, overcurrent_status = {overcurrent_status}")
        print(f"slot_3-4-5-6-7_8_overcurrent = {slot_overcurrent}")    
        
        command = str(input("Disable slot? y/n "))
        if command == "y":
            slot_controle(slot,"disable")
        #disable_all_slots()
            
        


except KeyboardInterrupt:
    pass




