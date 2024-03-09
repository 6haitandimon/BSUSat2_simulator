from os import stat
from math import ceil
from device_controller.device_config import SlotCommunicationProtocol
from radio_driver.radio_controller import (
    RadioController
)
import uasyncio


FILE_PAGE_SIZE = 160

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


def send_file_data(self, transfer_config: FileTransferConfig) -> int:
    for page_idx in range(transfer_config.page_count):
        self.send_page(transfer_config, page_idx)
    return None


path = "tests/test_message.txt"

def build_page_send_msg(data, idx):
    msg = {
        SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_FILE_PAGE_VALUE,
        SlotCommunicationProtocol.SLOT_PAGE_INDEX_KEY: idx,
        SlotCommunicationProtocol.SLOT_PAGE_DATA_KEY: data
    }
    return msg

if __name__ == "__main__":
    file_transfer_config = FileTransferConfig(path)
    radio_controller = RadioController()
    radio_controller.initialize()
    radio_controller.start_tx()
    opening_message = {
        SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_FILE_TRANSFER_INIT_VALUE,
        "file.name": file_transfer_config.name,
        "page.count": file_transfer_config.page_count
    }
    radio_controller.send_message("BSUSat", "BSUGS", opening_message)
    for page_idx in range(file_transfer_config.page_count):
        bytes_to_read = File.calc_available_page_bytes(file_transfer_config, page_idx)
        page_data = File.get_page_data(bytes_to_read, page_idx, file_transfer_config.os_path)
        page_message = build_page_send_msg(page_data, page_idx)
        radio_controller.send_message("BSUSat", "BSUGS",page_message)

    ending_message = {
        SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_FILE_TRANSFER_END_VALUE,
        "file.name": file_transfer_config.name,
        "page.count": file_transfer_config.page_count
    }
    radio_controller.send_message("BSUSat", "BSUGS", ending_message)




