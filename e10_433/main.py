import gc
import time
import uasyncio
from device_controller.slot_controller import SlotController
from machine import Pin, I2C
from tests import test_radio_send


def start_led_opening():
    led = Pin("LED", Pin.OUT)
    for i in range(3):
        led.value(1)
        time.sleep(0.5)
        led.value(0)
        time.sleep(0.5)


async def main():
    print('started main')
    slot_controller = SlotController()
    slot_controller.initialize()
    
    i2c = I2C(0, scl=Pin(21), sda=Pin(20), freq=400000)
    
    # TEST SEND
    await test_radio_send.test_radio_send(i2c)
    # print("govno")
    # TEST RECEIVE
    # test_radio_receive.test_radio_receive()

    start_led_opening()
    watchdog_refresh_counter = 0

    while True:
        await uasyncio.sleep_ms(2000)
        watchdog_refresh_counter += 1
        if watchdog_refresh_counter % 5 == 0:
            print("Free RAM memory : {}".format(gc.mem_free()))


if __name__ == "__main__":
    uasyncio.run(main())
