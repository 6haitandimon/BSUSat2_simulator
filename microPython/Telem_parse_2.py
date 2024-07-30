import socket
from construct import *
import time

UDP_IP = "127.0.0.1"
UDP_PORT = 9999

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(0.5)

# data=bytes.fromhex('4a0c8240a913d03d4a0c8240a913d03d26bf71c14a0c8240a913d03dc1ca8140cdaacf3d26bf71c16f12033c17b751b96f12033c17b751b98b6c673fa223b9bc6f12033c17b751b9b175213f7c420a3fec20814000000000000000000000000000000000000000000000000000000000ae47e941d94e69420ad7a33c000000006f12833c000000000ad7a33c00000000a69bc43c000000004260e53c000000007368814000000000a69bc43c00000000000080470000000000000000')

ff=Struct(
    "ina3221_1_1_voltage"/Int16ul, #0
    "ina3221_1_1_current"/Int16ul,
    "ina3221_1_3_voltage"/Int16ul,
    "ina3221_1_3_current"/Int16ul,
    "ina3221_1_ut1"/Int16ul,#----#4
    "ina3221_1_utsh1"/Int16ul,#----#4

    "ina3221_2_1_voltage"/Int16ul,
    "ina3221_2_1_current"/Int16ul,
    "ina3221_2_3_voltage"/Int16ul,
    "ina3221_2_3_current"/Int16ul,
    "ina3221_2_Ut2"/Int16ul,#----#9
    "ina3221_2_Utsh2"/Int16ul,#----#9

    "time"/Int32ul,
    "mcu_reset"/Int16ul,
    "mcu_reset_timestamp"/Int32ul,
    "mcu_load_cpu"/Int16ul,
    "mcu_clocks"/Int16ul,
    "mcu_temp"/Int16ul,
    "rtc_unix"/Int32ul,#------27

    "vSolraBus"/Int16ul,
    "vusb"/Int16ul,
    "Vbat"/Int16ul,#------  20

    "VSys"/Int16ul,
    "ISys"/Int16ul,

    
    "slot1Voltage"/Int16ul,
    "slot1Current"/Int16ul,#------29
    
    "slot2Voltage"/Int16ul,
    "slot2Current"/Int16ul,#------29

    "slot3Voltage"/Int16ul,
    "slot3Current"/Int16ul,#------31

    "slot4Voltage"/Int16ul,
    "slot4Current"/Int16ul,#------33

    "slot5Voltage"/Int16ul,
    "slot5Current"/Int16ul,#------35

    "slot6Voltage"/Int16ul,
    "slot6Current"/Int16ul,#------37
    
    "slot7Voltage"/Int16ul,
    "slot7Current"/Int16ul,#------37

    "slot8Voltage"/Int16ul,
    "slot8Current"/Int16ul,#------39

    "slot9Voltage"/Int16ul,
    "slot9Current"/Int16ul,#------41

    "slot10Voltage"/Int16ul,
    "slot10Current"/Int16ul,#------43

    "solarYminus_voltage"/Int16ul,
    "solarYminus_current"/Int16ul,#-----11 

    "solarYplus_voltage"/Int16ul,
    "solarYplus_current"/Int16ul,#-----13

    "solarXminus_voltage"/Int16ul,
    "solarXminus_current"/Int16ul,#------15

    "solarXplus_voltage"/Int16ul,
    "solarXplus_current"/Int16ul,#------17
    
    "solarZminus_voltage"/Int16ul,
    "solarZminus_current"/Int16ul,#------15

    "solarZplus_voltage"/Int16ul,
    "solarZplus_current"/Int16ul,#------17
   
    "enableSlot_1_10"/Int16ul,
    "OverCurrent"/Int16ul,
    "OverCurrentCounter"/Int16ul

)

# print(data)

# print(ff.parse(data[:]))


def main():
    while True:
        try:
            data, addr = sock.recvfrom(256) # buffer size is 1024 bytes
           
            RawTelemetry = bytes.fromhex("".join('{:02x}'.format(x) for x in data))
            # print(RawTelemetry)
            # lengthPacket = Int8ul.parse(RawTelemetry[:])
            
            print(ff.parse(RawTelemetry[16:]))
            print(time.strftime('%H:%M:%S'))
        except socket.timeout:
            pass

main()
