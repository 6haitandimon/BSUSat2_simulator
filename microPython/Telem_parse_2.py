# -*- coding: utf-8 -*-
"""
Created on Sat May 13 08:26:10 2023

@author: Vasilina
"""
from construct import*


# file = open("D:/PROJECTS/bsusat2_tm_example_r1/BSUSAT2_TRX_TelemetryDemo_2023.05.01/iq_f32.bin", "rb")
# data = file.read()

data=b'\xab\xa8\x91\xa3\xac\x15\xe9\xf8\xb0c\xa5\x07\x11\xa2>)\xe9qp\xf9\x99\xcf\xf0l\xec+\xa6\xe4\x08x5\xdd\x13\x8b\xfb\xb9\xb0\r\rs\x8b\x02\xbcBZ\xb7L\x8f'

data3=b'\xab\xa8\x91\xa3\xac\x15\xe9\xf8\xb0c\xa5\x07\x11\xa2>)\xe9qp\xf9\x99\xcf\xf0l\xec+\xa6\xe4\x08x5\xdd\x13\x8b\xfb\xb9\xb0\r\rs\x8b\x02\xbcBZ\xb7L\x8fN\x93'
data2=bytes.fromhex('aba891a3ac15e9f8b063a50711a23e29e97170f999cff06cec2ba6e4087835dd138bfbb9b00d0d738b02bc425ab74c8f')

data4=b'\xc7\x13Q\x93\xc4\x04\x04\xa4\x9a@\xc4\x04\xddW\x82@\xc4\x04\xef\xd2\xb1>u\x01'

data4=bytes.fromhex('b00d0d738b02bc425ab74c8f')

data6=b'\xa2>)\xe9qp\xf9\x99\xcf\xf0l\xec+\xa6\xe4\x08x5\xdd\x13'

data7=bytes.fromhex('789ce33eec208140d34d8240857cd03d4a0c8240a913d03d4a0c8240a913d03d4a0c8240a913d03d26bf71c126bf71c1')

b8=b'n\xdb\x99@\xddC\x82@\xe0C\x9f>'

ff=Struct(
    "vusb"/Float32l,
    "vbus"/Float32l,
    "ina3221_1_1_voltage"/Float32l,
    "ina3221_1_1_current"/Float32l,
    "ina3221_1_3_voltage"/Float32l,
    "ina3221_1_3_voltage"/Float32l,
    "ina3221_2_1_voltage"/Float32l,
    "ina3221_2_1_current"/Float32l,
    "ina3221_2_3_voltage"/Float32l,
    "ina3221_2_3_current"/Float32l,
    "ina3221_1_temp"/Float32l,
    "ina3221_2_temp"/Float32l
)

data1=b'\xc7\x13Q\x93'

StringMsg = Struct(
    'Message' / Bytes(3)
)

#print(StringMsg.parse(data1[:]))
print(ff.parse(data7[:]))
