from time import sleep
from machine import Pin, I2C

def main():
    i2c = I2C(0, scl=Pin(21), sda=Pin(20), freq=400000)
    print('Scan i2c bus...')
    while True:
        devices = i2c.scan()
        for D in devices:
            print(hex(D))
        sleep(2)

if __name__ == "__main__":
    main()