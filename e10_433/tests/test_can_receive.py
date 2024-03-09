from can_driver import ERROR
from can_driver.can.can_controller  import (
    CANController
)

from device_controller.device_config import (
    SlotCanAddressTable
)
import uasyncio
import gc


async def test_can_receive():
    receive_can_addr = SlotCanAddressTable.CAN_SEND_TEST_MODULE_ADDRESS
    print(f"CAN RECEIVE TEST\nMODULE CAN ADDRESS : {receive_can_addr}")
    print(f"Be sure some other device is also connected to CAN-BUS and bus terminator is present\n\r")
    can_controller = CANController()
    if not can_controller.initialize(receive_can_addr):
        print(f"CAN Controller Init failed")
        return False

    watchdog_refresh_counter = 0
    while True:
        await uasyncio.sleep_ms(2000)
        watchdog_refresh_counter += 1
        if watchdog_refresh_counter % 5 == 0:
            print("Free RAM memory : {}".format(gc.mem_free()))

if __name__ == "__main__":
    uasyncio.run(test_can_receive())

