
from construct import*

data=bytes.fromhex('4a0c8240a913d03d4a0c8240a913d03d26bf71c14a0c8240a913d03dc1ca8140cdaacf3d26bf71c16f12033c17b751b96f12033c17b751b98b6c673fa223b9bc6f12033c17b751b9b175213f7c420a3fec20814000000000000000000000000000000000000000000000000000000000ae47e941d94e69420ad7a33c000000006f12833c000000000ad7a33c00000000a69bc43c000000004260e53c000000007368814000000000a69bc43c00000000000080470000000000000000')

ff=Struct(
    "ina3221_1_1_voltage"/Float32l, #0
    "ina3221_1_1_current"/Float32l,
    "ina3221_1_3_voltage"/Float32l,
    "ina3221_1_3_current"/Float32l,
    "ina3221_1_temp"/Float32l,#----#4

    "ina3221_2_1_voltage"/Float32l,
    "ina3221_2_1_current"/Float32l,
    "ina3221_2_3_voltage"/Float32l,
    "ina3221_2_3_current"/Float32l,
    "ina3221_2_temp"/Float32l,#----#9

    "solarYminus_voltage"/Float32l,
    "solarYminus_current"/Float32l,#-----11 

    "solarYplus_voltage"/Float32l,
    "solarYplus_current"/Float32l,#-----13

    "solarXminus_voltage"/Float32l,
    "solarXminus_current"/Float32l,#------15

    "solarXplus_voltage"/Float32l,
    "solarXplus_current"/Float32l,#------17

    "vSolraBus"/Float32l,
    "vusb"/Float32l,
    "Vbat"/Float32l,#------  20

    "VSys_mcu"/Float32l, 
    "mcu_reset"/Float32l,
    "mcu_load_cpu"/Float32l,
    "mcu_ram"/Float32l,
    "mcu_clocks"/Float32l,
    "mcu_temp"/Float32l,
    "rtc_unix"/Float32l,#------27

    "slot1Voltage"/Float32l,
    "slot1Current"/Float32l,#------29

    "slot3Voltage"/Float32l,
    "slot3Current"/Float32l,#------31

    "slot4Voltage"/Float32l,
    "slot4Current"/Float32l,#------33

    "slot5Voltage"/Float32l,
    "slot5Current"/Float32l,#------35

    "slot6Voltage"/Float32l,
    "slot6Current"/Float32l,#------37

    "slot8Voltage"/Float32l,
    "slot8Current"/Float32l,#------39

    "slot9Voltage"/Float32l,
    "slot9Current"/Float32l,#------41

    "slot10Voltage"/Float32l,
    "slot10Current"/Float32l,#------43

    "enableSlot1:"/Int8ul,
    "enableSlot8:"/Int8ul,
    "enableSlot9:"/Int8ul,
    "enableSlot10:"/Int8ul,#------44
    
    "enableSlot3:"/Int8ul,
    "enableSlot4:"/Int8ul,
    "enableSlot5:"/Int8ul,
    "enableSlot6:"/Int8ul#------45
)

print(data)

print(ff.parse(data[:]))
