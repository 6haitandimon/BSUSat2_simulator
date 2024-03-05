from can_driver import ERROR
from can_driver.can.can_controller  import (
    CANController
)

from device_controller.device_config import (
    SlotCanAddressTable
)
import uasyncio
import umsgpack

from device_controller.device_config import DeviceConfig


async def can_packet_sender(driver, addr, data):
    while True:
        error = driver.send_message(DeviceConfig.INTERNAL_SLOT_ADDRESS, addr, data)
        if error == ERROR.ERROR_OK:
            print("TX  {}".format(data))
        else:
            print("TX Failed")
        await uasyncio.sleep_ms(1500)


async def test_can_send():
    target_can_addr = SlotCanAddressTable.CAN_SEND_TEST_MODULE_ADDRESS
    sample_can_data = [0x8E, 0x87, 0x32, 0xFA, ]
    print(f"CAN SEND TEST\nMODULE CAN ADDRESS : {DeviceConfig.INTERNAL_SLOT_ADDRESS}\nTARGET CAN ADDRESS : {target_can_addr}")
    print(f"Be sure some other device is also connected to CAN-BUS and bus terminator is present\n\r")
    can_controller = CANController()
    if not can_controller.initialize(DeviceConfig.INTERNAL_SLOT_ADDRESS):
        print(f"CAN Controller Init failed")
        return False
    print(f"Using following bytearray to send into CAN bus: {sample_can_data}")
    task = uasyncio.create_task(can_packet_sender(can_controller, target_can_addr, sample_can_data))

    await uasyncio.sleep_ms(6000)
    task.cancel()

if __name__ == "__main__":
    uasyncio.run(test_can_send())


