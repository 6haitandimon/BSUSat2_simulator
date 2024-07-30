from construct import *
import socket
# import time

UDP_IP = "127.0.0.1"
UDP_PORT = 9999

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(0.5)

ff=Struct(
    "data1"/Int8ul, 
    "data2"/Int8ul, 
    "data3"/Int8ul, 
    "data4"/Int8ul, 
    "data5"/Int8ul 
    )
def main():
    while True:
        try:
            data, addr = sock.recvfrom(256) # buffer size is 1024 bytes
           
            RawTelemetry = bytes.fromhex("".join('{:02x}'.format(x) for x in data))
            # print(RawTelemetry)
            # lengthPacket = Int8ul.parse(RawTelemetry[:])
            
            print(ff.parse(RawTelemetry[:]))
            # print(time.strftime('%H:%M:%S'))
        except socket.timeout:
            pass

main()
