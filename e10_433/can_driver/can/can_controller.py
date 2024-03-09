from .mcp2515 import CAN
from .can import CANFrame
from os import stat
from math import ceil
from uasyncio import create_task
from .can_message_parser import CANMessageParser, WrongFrameAddressException
from machine import Pin
from time import sleep_ms
import umsgpack


from device_controller.device_config import (
    InternalSPIConfig,
    DeviceConfig,
)


from can_driver import (
    SPI,
    CAN_CLOCK,
    CAN_SPEED,
    ERROR,
)
from . import (
    SEND_FILE_CFG_CMD_ID,
    MAX_SIMPLE_CAN_MSG_SIZE_BYTES,
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
    ERROR,
)

CAN_FRAME_CAPACITY = 8
CAN_CONTROLLER_INIT_FAILED = -2

#TODO: Create a separate class to handle all the FILE Stuff
class FileTransferConfig:
    def __init__(self, file_path):
        self._file_path = file_path
        self._file_name = file_path.split("/").pop()
        self._file_size_in_bytes = stat(file_path)[6]
        self._file_page_count = ceil(self._file_size_in_bytes / FILE_PAGE_SIZE)

    @property
    def name(self):
        return self._file_name

    @property
    def size(self):
        return self._file_size_in_bytes

    @property
    def page_count(self):
        return self._file_page_count

    @property
    def os_path(self):
        return self._file_path



class File:
    @staticmethod
    def get_page_data(bytes_to_read, page_idx, file_path):
        page_start = page_idx * FILE_PAGE_SIZE
        src_file = open(file_path, "rb")
        src_file.seek(page_start)
        data_to_send = src_file.read(bytes_to_read)
        src_file.close()
        return data_to_send

    @staticmethod
    def calc_available_page_bytes(transfer_config: FileTransferConfig, page_idx: int):
        if page_idx == transfer_config.page_count - 1:
            bytes_to_read = transfer_config.size % FILE_PAGE_SIZE
        else:
            bytes_to_read = FILE_PAGE_SIZE
        return bytes_to_read

class CANController:
    _initialized = False
    _instance = None
    _can_listener_task_handler = None

    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            cls._instance = super(CANController, cls).__new__(cls, *args, **kwargs)
        return cls._instance

    # Do we need async because of call of create_task from asyncio
    def initialize(self, can_addr: int = DeviceConfig.INTERNAL_SLOT_ADDRESS) -> False:
        if not self._initialized:
            self._init_data_control_fields(can_addr)

            ret = self._init_can_driver()
            if ret == CAN_CONTROLLER_INIT_FAILED:
                print("CAN Controller initialization failed")
                return False
            self.initialized = True
            return True

    @property
    def initialized(self):
        return self._initialized

    @initialized.setter
    def initialized(self, value):
        self._initialized = value

    def _init_can_driver(self):
        if self._can_driver.reset() != ERROR.ERROR_OK:
            print("Can not reset for MCP2515")
            return CAN_CONTROLLER_INIT_FAILED
        # Have to be 8MHZ if using in pair with MCP2515 (20MHZ with MCP25625)
        if self._can_driver.setBitrate(CAN_SPEED.CAN_500KBPS, CAN_CLOCK.MCP_8MHZ) != ERROR.ERROR_OK:
            print("Can not set bitrate for MCP2515")
            return CAN_CONTROLLER_INIT_FAILED
        if self._can_driver.setNormalMode() != ERROR.ERROR_OK:
            print("Can not set normal mode for MCP2515")
            return CAN_CONTROLLER_INIT_FAILED
        self._can_driver.enable_interrupt()
        self._can_listener_task_handler = create_task(self.handle_can_msg())

    def _init_data_control_fields(self, can_addr):
        self._can_self_addr = can_addr
        cmd_id = START_PAGE_TRANSFER_CMD_ID
        can_id = CANFrame.build_frame_id(cmd_id)
        self.init_page_send_can_msg = CANFrame(can_id=can_id, data=None)
        cmd_id = END_PAGE_TRANSFER_CMD_ID
        can_id = CANFrame.build_frame_id(cmd_id)
        self.end_page_send_can_msg = CANFrame(can_id=can_id, data=None)
        self._can_driver = CAN(SPI(cs=InternalSPIConfig.SPI_PICO_CS_PIN))
        self._can_parser = CANMessageParser(can_addr)

    async def handle_can_msg(self):
        print("CAN Controller frame handler started")
        while True:
            await self._can_driver.can_rx_tsf.wait()
            error, can_frame = self._can_driver.readMessage()
            self._can_driver.clearInterrupts()
            print('Triggered : {} {}'.format(error, can_frame))
            if error == ERROR.ERROR_OK:
                print("Delegate frame parse to can_parser")
                try:
                    await self._can_parser.parse_frame(can_frame)
                except WrongFrameAddressException:
                    print("Parse failed")
                finally:
                    print("Parse ended")
            else:
                print("can_driver RX Failed")

            # Could set an Event here to trigger multiple tasks

    def send_message(self, src_addr: int, dst_addr: int, payload: list):

        if len(payload) >= MAX_SIMPLE_CAN_MSG_SIZE_BYTES:
            print("Message is too big to be transfered via CAN. Better to pack it as file.")
            return ERROR.ERROR_FAIL

        can_packet = self.build_can_pkt(payload)

        if len(can_packet) > CHUNK_DATA_SIZE:
            print("Message is going to be split. Message : {}".format(can_packet))
            self._split_and_send(data=can_packet, src_addr=src_addr, dst_addr=dst_addr)
        else:
            message_frame_id = CANFrame.build_frame_id(cmd_id=SEND_FREE_FRAME_CMD_ID, dst_addr=dst_addr, src_addr=src_addr)
            frame = CANFrame(can_id=message_frame_id, data=can_packet)
            self._can_driver.sendMessage(frame)
        return ERROR.ERROR_OK

    def init_page_sending(self):
        # send_frame
        self._can_driver.sendMessage(self.init_page_send_can_msg)

    def end_page_sending(self):
        # send_frame
        self._can_driver.sendMessage(self.end_page_send_can_msg)

    @staticmethod
    def build_can_pkt(payload: list) -> list:
        packed_data = umsgpack.dumps(payload)
        if type(packed_data) != bytearray:
            packed_data = bytearray(packed_data)
        return packed_data


    # TODO: Delegate such methods as one below to parser?
    @staticmethod
    def build_file_transfer_cfg_msg(pending_file_name: str, pending_file_size: int,
                                    side_comment: str = None) -> bytearray:
        cfg_dict = {
            MSGPACK_COMMAND_LITERAL_TOKEN: FILE_TRANSFER_CFG_CMD,
            MSGPACK_FILENAME_LITERAL_TOKEN: pending_file_name,
            MSGPACK_FILE_SIZE_LITERAL_TOKEN: pending_file_size,
        }
        if side_comment is not None:
            cfg_dict.update({MSGPACK_COMMENT_LITERAL_TOKEN: side_comment})
        return umsgpack.dumps(cfg_dict)

    def transfer_file(self, file_path: str) -> int:
        transfer_config = FileTransferConfig(file_path)
        print(f"{transfer_config.name=}")
        self.send_file_transfer_cfg(transfer_config)
        self.send_file_data(transfer_config)

    def send_file_data(self, transfer_config: FileTransferConfig) -> int:
        for page_idx in range(transfer_config.page_count):
            self.send_page(transfer_config, page_idx)
        return None


    def send_page(self, transfer_config: FileTransferConfig, page_idx: int):
        self.init_page_sending()
        # Maybe Better to calculate page bytes before transfer of whole file started
        bytes_to_read = File.calc_available_page_bytes(transfer_config, page_idx)
        page_data = File.get_page_data(bytes_to_read, page_idx, transfer_config.os_path)
        self._split_and_send(page_data, cmd_id=SEND_PAGE_CMD_ID)
        self.end_page_sending()

    def send_file_transfer_cfg(self, transfer_config: FileTransferConfig):
        cfg_message = self.build_file_transfer_cfg_msg(transfer_config.name, transfer_config.size)
        self._split_and_send(cfg_message, cmd_id=SEND_FILE_CFG_CMD_ID)
        return ERROR.ERROR_OK

    def _split_and_send(self, data: bytearray, cmd_id: int = SEND_FREE_FRAME_CMD_ID, src_addr: int = 0x5, dst_addr: int = 0x3) -> int:
        ret = ERROR.ERROR_OK
        chunk_count = len(data) // 8

        if len(data) % 8 != 0:
            chunk_count += 1
        for chunk_idx in range(chunk_count):


            frame_start = chunk_idx * 8
            frame_end = (chunk_idx + 1) * 8 if chunk_idx != chunk_count - 1 else len(data)

            chopped_data = data[frame_start:frame_end]

            if chunk_idx == chunk_count - 1:
                chunk_idx |= 0x800

            can_id = CANFrame.build_frame_id(cmd_id, chunk_id=chunk_idx, src_addr=src_addr, dst_addr=dst_addr)
            can_msg_frame = CANFrame(can_id=can_id, data=chopped_data)
            # send_can_msg(can_msg_frame)
            # TODO: Add exception for failed message transmit
            print("Sending following frame : {}".format(can_msg_frame))
            ret = self._can_driver.sendMessage(can_msg_frame)
            print("RET = {}".format(ret))
            sleep_ms(35)
            if ret != ERROR.ERROR_OK:
                print("Message sent failed, trying again")
                ret = self._can_driver.sendMessage(can_msg_frame)
                if ret != ERROR.ERROR_OK:
                    print("Message failed sent again, stopping the process")
                    return ret
                else:
                    print("Resent succeeded")
            
        return ret
            # parser.parse(can_msg_frame)



