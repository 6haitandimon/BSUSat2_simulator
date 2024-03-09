from builtins import round

from .can import CANFrame
from device_controller import command_queue
import umsgpack
from math import ceil

FRAME_PARSE_SUCESS = 200

PAGE_CONSISTENCY_ERROR = 401

try:
    import micropython
except ModuleNotFoundError:
    pass

try:
    from typing import Any, Optional, List, Tuple
except ImportError:
    pass

#errors classes
PAGE_ASSEMBLE_SUCCESS = 201

#msgpack tokens
from . import (
    SEND_FILE_CFG_CMD_ID,
    SEND_FILE_CMD_ID,
    SEND_FREE_FRAME_CMD_ID,
    START_PAGE_TRANSFER_CMD_ID,
    END_PAGE_TRANSFER_CMD_ID,
    SEND_PAGE_CMD_ID,
    FILE_TRANSFER_CFG_CMD,
    MSGPACK_COMMAND_LITERAL_TOKEN,
    MSGPACK_FILENAME_LITERAL_TOKEN,
    MSGPACK_FILE_SIZE_LITERAL_TOKEN,
    MSGPACK_COMMENT_LITERAL_TOKEN,
    MSGPACK_PARAMETER_LITERAL,
    MSGPACK_DATA_LITERAL,
    CHUNK_DATA_SIZE,
    FILE_PAGE_SIZE,
)


DST_ADDR_IDX = 0
COMMAND_IDX = 1
PARAMETER_IDX = 2
DATA_START_IDX = 3
DEFAULT_ADDR = 0x3

WRONG_COMMAND_ID_ERR = -5
FILE_PAGE_BUFFER = bytearray()


class WrongFrameAddressException(Exception):
    def __init__(self, msg):
        super().__init__(msg)

class CANMessageParser:
    CAN_BUFFER_ZERO = dict()
    CAN_FILE_BUFFER = dict()
    CAN_PAGE_BUFFER = dict()
    receiving_file_config = dict()
    file_buffered_pages_count = 0
    page_buffer = bytearray()

    def __init__(self, module_addr: int):
        self._module_addr = module_addr
        self._last_msg_id = -1

    def is_frame_dst_addr_valid(self, frame: CANFrame):
        return self._module_addr == frame.dst_addr

    def handle_message(self, msg: dict):
        if msg.pop(MSGPACK_COMMAND_LITERAL_TOKEN) == FILE_TRANSFER_CFG_CMD:
            self.CAN_FILE_BUFFER.update(msg)

    # @micropython.viper
    # def save_page(self, data):
    #     page_buf = ptr8(self.page_buffer)
    #     for i, key in enumerate(sorted(self.CAN_FILE_BUFFER.keys())):
    #         page_buf[i] = self.CAN_FILE_BUFFER.pop(key)
    #     with open("file_test.txt", "a") as can_file:
    #         can_file.write(self.page_buffer)

    @staticmethod
    def is_sharded_buffer_consistent(buffer: dict) -> bool:
        prev_idx = -1
        consistency_flag = True
        for chunk_idx in sorted(buffer.keys()):
            if chunk_idx != prev_idx + 1:
                print("Broken page")
                consistency_flag = False
                # locate bad index ?
                break
            prev_idx = chunk_idx
        return consistency_flag

    def collect_received_page_data(self, page: dict) -> None:
        # Maybe it's better to write all data directly to page file and construct the result out of all pages (files)?
        for chunk_index in sorted(page.keys()):
            self.page_buffer += page.pop(chunk_index)
        with open(self.receiving_file_config[MSGPACK_FILENAME_LITERAL_TOKEN], "ab") as recv_image:
            recv_image.write(self.page_buffer)
        self.page_buffer.clear()

    @staticmethod
    def create_recv_file_storage(self, storage_name: str) -> None:
        storage = open(storage_name, "wb")
        storage.close()

    async def parse_frame(self, message_frame: CANFrame) -> Tuple[int, Any]:
        print("Trying to parse can_frame : {}".format(message_frame))
        if self.is_frame_dst_addr_valid(message_frame):
            if message_frame.cmd_idx == SEND_FILE_CFG_CMD_ID:
                print("Parsing file transfer configuration")
                self.compose_file_cfg(message_frame)
                return FRAME_PARSE_SUCESS, None
            elif message_frame.cmd_idx == START_PAGE_TRANSFER_CMD_ID:
                print("Started page transfer")
                return FRAME_PARSE_SUCESS, None
            elif message_frame.cmd_idx == END_PAGE_TRANSFER_CMD_ID:
                print("Ended page_transfer")
                res = self.assemble_received_page()
            elif message_frame.cmd_idx == SEND_PAGE_CMD_ID:
                print("File page frame")
                self.CAN_PAGE_BUFFER.update({message_frame.chunk_idx: message_frame.data})
            elif message_frame.cmd_idx == SEND_FREE_FRAME_CMD_ID:
                print("Free frame")
                await self.process_free_frame(message_frame)
            else:
                print("Undefined command index")
        else:
            raise WrongFrameAddressException(f"Frame dst address({message_frame.dst_addr}) is differs from module address({self._module_addr})")

    async def process_free_frame(self, message_frame):
        self._last_msg_id = message_frame.cmd_idx

        if message_frame.cmd_idx not in self.CAN_BUFFER_ZERO.keys():
            self.CAN_BUFFER_ZERO.update(
                {message_frame.chunk_idx: message_frame.data}
            )
        # TODO : ADD check for cmd_idx identity in case of message with new id when the sequenced message still in charge
        # Solution : Make a src_id based buffer for received messages

        if message_frame.is_chunk_last:
            if self.is_sharded_buffer_consistent(self.CAN_BUFFER_ZERO):
                #create a task instead of waiting for message assemble
                resulted_message = await self.assemble_resulting_message()
                return resulted_message
            else:
                return None

        else:
            print('buffer filled, wait for other chunk')
            return None

    def get_lost_idxs(self, idx_list):
        available_idxs = idx_list
        lost_idxs = list()
        for i in range(len(available_idxs) - 1):
            step = available_idxs[i + 1] - available_idxs[i]
            if step != 1:
                if step == 2:
                    lost_idxs.append(available_idxs[i] + 1)
                else:
                    lost_idxs.extend(
                        [available_idxs[i] + j for j in range(1, available_idxs[i + 1] - available_idxs[i])])
        return lost_idxs

    async def assemble_resulting_message(self):
        print('encoding buffer')
        # print(self.CAN_BUFFER_ZERO)
        # TODO: Delegate msgpack deserialize to message_parser on top layer
        lost_idxs = self.get_lost_idxs(sorted(self.CAN_BUFFER_ZERO.keys()))
        if lost_idxs:
            print(f"trying to get back new idxs : {lost_idxs}")
            # upfill_parted_buffer() WIP

        resulted_message = bytearray()
        for chunk_index in sorted(self.CAN_BUFFER_ZERO.keys()):
            resulted_message += self.CAN_BUFFER_ZERO.pop(chunk_index)
        # print("res : {}".format(resulted_message))
        # parsed_message = umsgpack.loads(resulted_message)
        await command_queue.put(resulted_message)
        # self.handle_message(parsed_message)
        # print(f"Parser result : {parsed_message}")
        self.CAN_BUFFER_ZERO.clear()
        return resulted_message

    def assemble_received_page(self) -> int:

        if not self.is_sharded_buffer_consistent(self.CAN_PAGE_BUFFER):
            print("page transmission error, getting back to clear state")
            self.broken_page_idxs = self.file_buffered_pages_count + 1
            self.receiving_file_config.clear()
            self.page_buffer.clear()
            return PAGE_CONSISTENCY_ERROR

        self.collect_received_page_data(self.CAN_PAGE_BUFFER)
        self.file_buffered_pages_count += 1
        if self.file_buffered_pages_count == ceil(
                self.receiving_file_config[MSGPACK_FILE_SIZE_LITERAL_TOKEN] / FILE_PAGE_SIZE):
            print("Wrapping up the file")
            self.receiving_file_config.clear()

    def compose_file_cfg(self, message_frame):
        self.CAN_BUFFER_ZERO.update(
            {message_frame.chunk_idx: message_frame.data}
        )
        if message_frame.is_chunk_last:
            print('encoding file cfg buffer')
            chunk_buffer = bytearray()
            for chunk_idx in sorted(self.CAN_BUFFER_ZERO.keys()):
                chunk_buffer += self.CAN_BUFFER_ZERO.pop(chunk_idx)
            self.receiving_file_config.update(umsgpack.loads(chunk_buffer))
            # self.create_recv_file_storage(self.receiving_file_config[MSGPACK_FILENAME_LITERAL_TOKEN])
            self.create_recv_file_storage("Input.png")

    def __str__(self) -> str:
        return f"Buffer len: {len(self.CAN_BUFFER_ZERO)}, Last message id : {self._last_msg_id}, CAN ID : {self._CAN_id}"

#
# parser = CANMessageParser(0x5)
#
# def test_parser():
#     from can import CANFrame as frame_model
#     parser = CANMessageParser(0xF)
#     listed_data = umsgpack.dumps({"cmd": "telemetry_send", "TSL1": [0x11, 0x22], "TSL2": [0x31, 0xFF]})
#     print(listed_data)
#     chopped_data = [listed_data[i * 8:(i + 1) * 8] for i in range(len(listed_data) // 8)]
#     if len(listed_data) % 8 != 0:
#         chopped_data.append(listed_data[(len(listed_data) - len(listed_data) % 8): len(listed_data)])
#     print(chopped_data)
#
#     cmd_id = 0x0B
#     chunk_sequence_id = 0x00
#     priority = 0b111
#     for chunk_count, frame_data in enumerate(chopped_data):
#         if chunk_count == len(chopped_data) - 1:
#             chunk_sequence_id |= 0x800
#         print(f"chunk_id : {chunk_sequence_id}")
#         can_id = 0x000000F3 | (chunk_sequence_id << 8) | (cmd_id << 20) | (priority << 26)
#         print('{} [{}]'.format(hex(can_id), ', '.join(str(x) for x in frame_data)))
#         abstrct_can_msg = frame_model(can_id=can_id, data=frame_data)
#         chunk_sequence_id += 1
#         parser.parse(abstrct_can_msg)

if __name__ == "__main__":
    print("Tests")
    # test_parser()
    # test_file_send()

