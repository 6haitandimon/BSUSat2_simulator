from device_controller.device_config import SlotCommunicationProtocol, DeviceConfig
from radio_driver.radio_controller import (
    RadioController
)
import uasyncio
from time import ticks_ms

TEST_RADIO_MESSAGE = {
    SlotCommunicationProtocol.STATION_TIMESTAMP_KEY: ticks_ms(),
    SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_TELEMETRY_CMD_VALUE,
    "VSolar": 5.5,
    "VBat1": 4.25,
    "VBat2": 3.75,
    "VSys": 4.0,
    "ISys": 170,
    "Temp": 24
}


async def radio_packet_sender(driver, message):
    while True:
        await driver.send_message(DeviceConfig.RADIO_MODULE_DEFAULT_AX25_ADDRESS,
                                  DeviceConfig.STATION_EMITATOR_DEFAULT_ADDRESS,
                                  message)
        await uasyncio.sleep_ms(3000)


async def test_radio_send():
    print('start sending test')
    radio_controller = RadioController()
    if not radio_controller.initialize():
        print(f"Radio Controller Init failed")
        return None
    radio_controller.start_tx()
    print(f"Radio state before sending sequence: {radio_controller.get_tranciever_state()}")
    print(f"Message to send : {TEST_RADIO_MESSAGE}")
    await radio_packet_sender(radio_controller, TEST_RADIO_MESSAGE)
    await uasyncio.sleep_ms(6000)
