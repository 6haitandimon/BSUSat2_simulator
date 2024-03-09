from machine import SPI
from micropython import const
from device_controller.device_config import InternalSPIConfig as SPI_CONFIG

SPI_DUMMY_INT = const(0x00)
SPI_TRANSFER_LEN = const(1)
SPI_DEFAULT_BAUDRATE = const(10000000)  # 10MHz
SPI_HOLD_US = const(50)
