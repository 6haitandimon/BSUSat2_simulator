from .AX25header import AX25Packet
from .encoder import decode_radio_message
from device_controller import command_queue
from device_controller.device_config import DeviceConfig


class RadioMessageParser:

    def __init__(self, sat_addr: str = DeviceConfig.RADIO_MODULE_DEFAULT_AX25_ADDRESS):
        self._sat_addr = sat_addr

    def parse(self, message):
        print(message)
        print("Parsing message")
        # decoding
        decoded_message = decode_radio_message(message)
        parsed_message = AX25Packet.parse(decoded_message)
        #         print("{} vs {}".format(parsed_message.dst_addr, self._sat_addr))
        #         if rc == ErrorTable.SAT_ERROR:
        #             print("Parse failed")
        #             return rc
        if parsed_message.dst_addr.get_call_sign() == self._sat_addr:
            #             print("Everything is ok, sending message to queue")
            await command_queue.put(parsed_message.payload)
            # slot_controller = SlotController()
            # slot_controller.command_queue.put(parsed_message.payload)
        else:
            print("Wrong destination address")
            with open("IM01.txt", "a") as wrong_msg_log:
                wrong_msg_log.write("".format(parsed_message))
        print("Parse ended")
