import gc
import time
import uasyncio
from device_controller.slot_controller import SlotController
from machine import Pin
from tests import test_radio_send, test_radio_receive


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

    # TEST SEND
    await test_radio_send.test_radio_send()

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
