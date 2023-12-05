from machine import Pin, UART, PWM

#Import Delaying libary utime 
import utime, time

#GPS Module UART Connection
gps_module = UART(1, baudrate=9600, tx=Pin(4), rx=Pin(5))

#print gps module connection details
print(gps_module)

#Used to Store gps data
buff = bytearray(255)

TIMEOUT = False

#store the status of handler is fixed or not
FIX_STATUS = False

#Store GPS Coordinates
latitude = ""
longitude = ""
satellites = ""
gpsTime = ""

#function to get gps Coordinates
def getPosData(gps_module):
    global FIX_STATUS, TIMEOUT, latitude, longitude, satellites, gpsTime
    
    #run while loop to get gps data
    #or terminate while loop after 5 seconds timeout
    # TIMEOUT - bool timeout - time
    timeout = time.time() + 8   # 8 seconds from now
    while True:
        gps_module.readline()
        buff = str(gps_module.readline())
        
        #b'$GNGGA,Time,Latitude,N/S,Longitude,E/W,Pos.fix,n sat,0.9,102.1,M,0.0,M,,*6C\r\n'
       
        parts = buff.split(',')
        
      
        #if no gps displayed remove "and len(parts) == 15" from below if condition is true
        if (parts[0] == "b'$GNGGA" and len(parts) == 15):
            if(parts[1] and parts[2] and parts[3] and parts[4] and parts[5] and parts[6] and parts[7]):
               
                
                latitude = convertToDegree(parts[2])
                # parts[3] contain 'N' or 'S'
                if (parts[3] == 'S'):
                    latitude = latitude[::-1]
                longitude = convertToDegree(parts[4])
                # parts[5] contain 'E' or 'W'
                if (parts[5] == 'W'):
                    longitude = longitude[::-1]
                satellites = parts[7]
                gpsTime = parts[1][0:2] + ":" + parts[1][2:4] + ":" + parts[1][4:6]
                FIX_STATUS = True
                break
                
        if (time.time() > timeout):
            TIMEOUT = True
            break
        utime.sleep_ms(500)
        
#function to convert raw Latitude and Longitude
#to actual Latitude and Longitude
def convertToDegree(RawDegrees):

    RawAsFloat = float(RawDegrees)
    firstdigits = int(RawAsFloat/100) #degrees
    nexttwodigits = RawAsFloat - float(firstdigits*100) #minutes
    
    Converted = float(firstdigits + nexttwodigits/60.0)
    Converted = '{0:.6f}'.format(Converted) # to 6 decimal places
    return str(Converted)
    
while True:
    
    getPosData(gps_module)

    #if gps data is found then print it on console
    if(FIX_STATUS == True):
        print("fix......")
      
        print(latitude)
        print(longitude)
        print(satellites)
        print(gpsTime)
        
        FIX_STATUS = False
        
    if(TIMEOUT == True):
        print("Request Timeout: No GPS data is found.")
        TIMEOUT = False

#____________________________LED_module__________________________________________

""" ir_led_pin = 0
ir_led = PWM(Pin(ir_led_pin, Pin.OUT))
ir_led.freq(33000)
ir_led.duty_u16(32786)

def send_pulse():
   ir_led.duty_u16(32786)
   time.sleep_us(560)
   ir_led.duty_u16(0)

message = "pico"
data = [ord(char) for char in message]

def send_data(data):
   for char in data:
       print(bin(char))
       for i in range(8):
           send_pulse()
           if char & (1 << i):
               time.sleep_us(1200)
           else:
               time.sleep_us(200) 
   time.sleep_ms(5)
   
def blink_led():
 for _ in range(1):
   ir_led.duty_u16(32786)
   time.sleep(1)
   ir_led.duty_u16(0)
   time.sleep(1)

for _ in range(3):
   send_data(data)
   blink_led()
   print(data)
   time.sleep(3) """
#_______________________________________________________________________________________