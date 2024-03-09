from device_controller.device_config import SlotCommunicationProtocol
from radio_driver.radio_controller import (
    RadioController
)
import uasyncio
from time import ticks_ms

def radio_message(i2c):
    vbat = i2c.readfrom_mem(0x41, 0, 4)
    vusb = i2c.readfrom_mem(0x41, 4, 4)
    vsolar = i2c.readfrom_mem(0x41, 8, 4)
    RADIO_MESSAGE = {vbat, vusb, vsolar}
    return RADIO_MESSAGE

async def radio_packet_sender(driver, i2c):
    while True:
        message = radio_message(i2c)
        task = uasyncio.create_task(driver.send_message("PICOS", "PICOWR", message))
        await uasyncio.sleep_ms(3000)


async def test_radio_send(i2c):
    radio_controller = RadioController()
    if radio_controller.initialize() != 0:
        print(f"Radio Controller Init failed")
        return None
    radio_controller.start_tx()
    print(f"Radio state before sending sequence: {radio_controller.get_tranciever_state()}")
    task = uasyncio.create_task(radio_packet_sender(radio_controller, i2c))
    await uasyncio.sleep_ms(6000)
