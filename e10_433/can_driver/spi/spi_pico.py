try:
    from typing import Any, Optional
except ImportError:
    pass

try:
    from pyb import Pin, SPI as MICROPYTHON_SPI
except ImportError:
    from machine import Pin, SPI as MICROPYTHON_SPI

from . import SPI_CONFIG
from .spi import SPI


class SPIPICO(SPI):
    def init(self, baudrate: int) -> Any:
        return MICROPYTHON_SPI(
            SPI_CONFIG.SPI_PICO_HARDWARE_CHANNEL,
            sck=Pin(SPI_CONFIG.SPI_PICO_SCK_PIN),
            mosi=Pin(SPI_CONFIG.SPI_PICO_MOSI_PIN),
            miso=Pin(SPI_CONFIG.SPI_PICO_MISO_PIN),
            baudrate=baudrate,
            firstbit=SPI_CONFIG.SPI_DEFAULT_FIRSTBIT,
            polarity=SPI_CONFIG.SPI_DEFAULT_POLARITY,
            phase=SPI_CONFIG.SPI_DEFAULT_POLARITY,
        )
