import umsgpack
from radio_driver.encoder import encode_radio_message
from radio_driver.crc import calculate_crc
from radio_driver.AX25header import create_AX25_address, create_AX25_packet
from radio_driver.radio_message_parser import RadioMessageParser
from machine import SPI, Pin
from radio_driver.SI4463 import SI4463
from device_controller.device_config import RadioHardwareConfig
import uasyncio

import time
import machine

try:
    from typing import Any
except ImportError:
    pass

### RADIO DEFINES ###
RADIO_MESSAGE_PREPARE_FAILED = -1


class RadioController:
    initialized = False
    _instance = None
    radio_listener_task_handler = None

    def __init__(self):
        if not self.initialized:
            cs_pin = Pin(RadioHardwareConfig.SPI_RADIO_CS_PIN, Pin.OUT)
            sdn_pin = Pin(RadioHardwareConfig.RADIO_SDN_PIN, Pin.OUT)
            sck_pin = Pin(RadioHardwareConfig.SPI_RADIO_SCK_PIN)
            mosi_pin = Pin(RadioHardwareConfig.SPI_RADIO_MOSI_PIN)
            miso_pin = Pin(RadioHardwareConfig.SPI_RADIO_MISO_PIN)
            irq_pin = Pin(RadioHardwareConfig.DEFAULT_RADIO_IRQ_PIN, Pin.IN)

            module_internal_spi = machine.SPI(RadioHardwareConfig.SPI_RADIO_HARDWARE_CHANNEL,
                                              baudrate=5000000,
                                              polarity=0,
                                              phase=0,
                                              bits=8,
                                              firstbit=machine.SPI.MSB,
                                              sck=sck_pin,
                                              mosi=mosi_pin,
                                              miso=miso_pin)
            #         #
            self._radio_tranciever_driver = SI4463(
                spi=module_internal_spi,
                sck_pin=sck_pin,
                miso_pin=miso_pin,
                mosi_pin=mosi_pin,
                cs_pin=cs_pin,
                sdn_pin=sdn_pin,
                irq_pin=irq_pin,
            )

            self.initialized = True

    def initialize(self):
        rc = self._radio_tranciever_driver.initialize()
        if rc != 0:
            print('radio_tranciever_driver.initialize: failed')

        rc = self._radio_tranciever_driver.clear_int_status()
        if rc != 0:
            print('radio_tranciever_driver.clear_int_status: failed')

        rc = self._radio_tranciever_driver.clear_fifo(clear_rx=True, clear_tx=True)
        if rc != 0:
            print('_radio_tranciever_driver.clear_fifo: failed')

        self._radio_parser = RadioMessageParser()
        self.radio_listener_task_handler = uasyncio.create_task(self.handle_radio_msg())

        return rc

    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            cls._instance = super(RadioController, cls).__new__(cls, *args, **kwargs)
        return cls._instance

    def get_tranciever_state(self):
        return self._radio_tranciever_driver.get_state()

    async def handle_radio_msg(self):
        while True:
            await self._radio_tranciever_driver.rx_tsf.wait()
            # TODO: Add handle for lack of space inside queue and item dropout as the result
            #             print("A")
            try:
                radio_message = self._radio_tranciever_driver.get_rx_packet()
                await self._radio_parser.parse(radio_message)
            except Exception as e:
                print(f"Exception : {e}")

            self._radio_tranciever_driver.clear_rx_buff()
            self._radio_tranciever_driver.set_state(8)
            print("Tranciever state : {}".format(self._radio_tranciever_driver.get_state()))

    def clear_driver_interrupts(self):
        return self._radio_tranciever_driver.clear_int_status(clear_ph=0xFF, clear_modem=0xFF, clear_chip=0xFF)

    def start_rx(self):
        return self._radio_tranciever_driver.start_rx()

    def full_clear_fifo(self):
        return self._radio_tranciever_driver.clear_fifo(clear_rx=True, clear_tx=True)

    def start_tx(self):
        return self._radio_tranciever_driver.set_ready_state()

    async def send_message(self, src_addr: str, dst_addr: str, payload: list) -> Any:
        try:
            print(f"payload to dump : {payload}")
            encoded_packet = self.build_radio_pkt(src_addr, dst_addr, payload)
            if self.get_tranciever_state() != 3:
                self.start_tx()
                print(f"Driver level")
            res = await self._radio_tranciever_driver.send_tx_packet(encoded_packet, 0)
            return res
        except Exception as e:
            print(f"Exception : {e}")
            return RADIO_MESSAGE_PREPARE_FAILED

    # Pack message as standardized AX.25
    @staticmethod
    def build_radio_pkt(src_addr: int, dst_addr: int, payload: list) -> list:
        packed_data = umsgpack.dumps(payload)
        ax25_packet = RadioController.wrap_with_AX25(dst_addr, src_addr, packed_data)
        encoded_packet = encode_radio_message(ax25_packet)
        return encoded_packet

    @staticmethod
    def wrap_with_AX25(dst_addr, src_addr, packed_data):
        AX25_src_addr = create_AX25_address(src_addr, 1, False)
        AX25_dst_addr = create_AX25_address(dst_addr, 0, True)
        ax25_packet = create_AX25_packet(AX25_src_addr, AX25_dst_addr, packed_data).encode()
        return ax25_packet


if __name__ == "__main__":
    try:
        RadioController.build_radio_pkt("BSUGS1234", "BSUIM", [0xAA for _ in range(20)])
    except Exception as e:
        print(f"Exception : {e}")
