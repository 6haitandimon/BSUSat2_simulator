from machine import Pin, PWM
import time

ir_led_pin = 0
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
   time.sleep(3)
