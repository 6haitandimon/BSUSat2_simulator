from utime import sleep_us, sleep_ms
from micropython import const
from radio_driver.radio_config import radio_config_array
from machine import Pin, SPI, disable_irq, enable_irq
from uasyncio import ThreadSafeFlag
import time


class SI4463API:
    NOP = const(0x00)
    PART_INFO = const(0x01)
    FUNC_INFO = const(0x10)
    SET_PROPERTY = const(0x11)
    GET_PROPERTY = const(0x12)
    GPIO_PIN_CFG = const(0x13)
    FIFO_INFO = const(0x15)
    PACKET_INFO = const(0x16)
    IRCAL = const(0x17)
    GET_MODEM_STATUS = const(0x22)
    PROTOCOL_CFG = const(0x18)
    GET_INT_STATUS = const(0x20)
    GET_PH_STATUS = const(0x21)
    GET_CHIP_STATUS = const(0x23)
    START_TX = const(0x31)
    START_RX = const(0x32)
    REQUEST_DEVICE_STATE = const(0x33)
    CHANGE_STATE = const(0x34)
    READ_CMD_BUFF = const(0x44)
    WRITE_TX_FIFO = const(0x66)
    READ_RX_FIFO = const(0x77)


TX_WRITE_BYTE_LIMIT = 24
SI446X_CMD_READ_CMD_BUFF = const(0x44)
SI446X_PKT_FIELD_2_LENGTH_LOW = ((0x12 << 8) | 0x12)

RET_FAIL = const(-1)
RET_OP_TIMEOUT = const(-2)

RET_SUCCESS = const(0)
RET_WRONG_CMD = const(1)
RET_NO_CTS = const(2)
RET_DEVICE_NOT_READY = const(3)

SCI_READY_SATE = const(3)
RADIO_STATE_RX = const(8)
RADIO_STATE_READY = const(3)

POWER_LEVEL_PROP = const(((0x22 << 8) | 0x01))

SI446X_CMD_FIFO_INFO = const(0x15)
SI446X_FIFO_CLEAR_RX = const(0x02)
SI446X_FIFO_CLEAR_TX = const(0x01)
RADIO_FIFO_THRESHOLD = const(0x30)
DEFAULT_CLEAR_PH = const(0x00)
DEFAULT_CLEAR_MODEM = const(0x00)
DEFAULT_CLEAR_CHIP = const(0x00)
UINT8_MAX = const(256)
RADIO_RX_MAX_PACKET_LENGTH = const(512)

PACKET_RX = const(0x10000000)
SYNC_DETECT = const(0x10000000000)
RX_FIFO_ALMOST_FULL = const(0x1000000)
PACKET_RX_PEND = const(0x100000)
FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = const(0x20000000000000)
TX_FIFO_ALMOST_EMPTY = const(0x2000000)
PACKET_SENT_PEND = const(0x200000)
CMD_ERROR_PEND = const(0x8000000000000)


class SI4463:
    """
          Construct an Si4463 object using already defined instances of PIN (sdn_pin, cs_pin) and SPI (spi) classes
          as basic parameters
    """

    # TODO: Decompose init function into several functions
    def __init__(self, spi: SPI, sdn_pin: Pin, sck_pin: Pin, miso_pin: Pin, mosi_pin: Pin, cs_pin: Pin,
                 irq_pin: Pin = None) -> None:
        self._spi = spi
        self._sdn_pin = sdn_pin
        self._sck_pin = sck_pin
        self._miso_pin = miso_pin
        self._mosi_pin = mosi_pin
        self._cs_pin = cs_pin
        self._irq_pin = irq_pin
        self.rx_tsf = ThreadSafeFlag()
        self.tx_tsf = ThreadSafeFlag()
        self.rx_blocked = False
        self.tx_block_avaliable = False
        self.rx_buff = bytes()
        # TODO: Add function to recreate ISR handler for current instance (with ability to change irq-related pin)

    def __del__(self):
        # disable IRQ
        if self._irq_pin is not None:
            self._irq_pin.irq(handler=None)
        print("SI4463 instance destroyed")

    def initialize(self):
        # setup isr for handling interrupts from chip
        self.si446x_chip_init()
        sleep_ms(300)
        rc = self.write_config()

        if self._irq_pin is not None:
            self._irq_pin.irq(trigger=Pin.IRQ_FALLING, handler=self.handle_radio_ISR)

        if rc == RET_SUCCESS:
            print("Write config sucess")
        else:
            self.__del__()
        print("Aloha")
        rc = self.start_rx()

        if rc == RET_SUCCESS:
            print("Start rx success")
        state = self.get_state()
        print(state)
        return rc

    def handle_radio_ISR(self, pin: Pin) -> None:
        print("radio irq")
        int_read = self.read_int_status()
        status = int.from_bytes(int_read, "little")
        self.handle_interrupt_status(status)

    def handle_TX_ISR(self, pin: Pin) -> None:
        self.tx_tsf.set()

    def si446x_chip_init(self) -> None:
        self._cs_pin.value(1)
        self._sdn_pin.value(1)
        sleep_us(10)
        self._sdn_pin.value(0)
        sleep_ms(5)

    def begin_comm(self) -> None:
        self._cs_pin.value(0)
        self._sck_pin.value(0)
        self._mosi_pin.value(0)

    def end_comm(self) -> None:
        self._mosi_pin.value(0)
        self._sck_pin.value(0)
        self._cs_pin.value(1)

    def is_send_cmd_allowed(self):
        rxdata = bytearray(2)
        self.begin_comm()
        self._spi.write_readinto(bytearray([SI4463API.READ_CMD_BUFF, 0xff]), rxdata)
        self.end_comm()
        #     print(f"is cmd allowed dbg {rxdata}")
        return list(rxdata)[1] == 0xFF

    def wait_for_cts(self):
        for i in range(100):
            if self.is_send_cmd_allowed():
                return True
            sleep_ms(10)
        print("NO CTS")
        return False

    def send_api_cmd(self, txdata, resp_len) -> list:
        txdata = bytearray(txdata)
        rxdata = bytearray(len(txdata))
        if len(txdata) == 0:
            return RET_WRONG_CMD
        if not self.wait_for_cts():
            return RET_NO_CTS
        self.begin_comm()
        self._spi.write_readinto(txdata, rxdata)
        self.end_comm()
        if resp_len == 0:
            return RET_SUCCESS
        if not self.wait_for_cts():
            return RET_NO_CTS
        #     print(f"before overslip : {rxdata}")
        self.begin_comm()
        self._spi.write(bytearray([SI4463API.READ_CMD_BUFF, 0xff]))
        response = self._spi.read(resp_len, 0x00)
        self.end_comm()
        return response

    def get_packet_info(self):
        self.begin_comm()
        self._spi.write(bytearray([SI4463API.PACKET_INFO]))
        self.end_comm()
        while not self.wait_for_cts():
            print("wait CTS")
        self.begin_comm()
        self._spi.write(bytearray([SI4463API.READ_CMD_BUFF, 0xff]))
        response = self._spi.read(2, 0x00)
        self.end_comm()
        #     print(f"{response=}")
        return response

    def clear_int_status(self, clear_ph: int = DEFAULT_CLEAR_PH, clear_modem: int = DEFAULT_CLEAR_MODEM,
                         clear_chip: int = DEFAULT_CLEAR_CHIP) -> int:
        intrp_stat_clear_cmd = [SI4463API.GET_INT_STATUS, clear_ph, clear_modem, clear_chip]
        ret = self.send_api_cmd(intrp_stat_clear_cmd, 0)
        if ret != RET_SUCCESS:
            return ret
        return RET_SUCCESS

    # def wait_response(resp_len):
    #     wait_for_cts()
    #     while True:
    #         begin_comm()
    #         cmd = [0x44, 0xff]

    # end_comm()

    def read_int_status(self) -> int:
        cmd = [SI4463API.GET_INT_STATUS, 0x00, 0x00, 0x00]
        int_response = self.send_api_cmd(cmd, 8)
        #     print(f"{int_response=}")
        return int_response

    def write_config(self) -> int:
        for i, cmd in enumerate(radio_config_array):
            rt = self.send_api_cmd(cmd, 0)
            if not rt == RET_SUCCESS:
                return rt
            rt = self.clear_int_status()
            if not rt == RET_SUCCESS:
                return rt
        return RET_SUCCESS

    def clear_fifo(self, clear_rx: bool, clear_tx: bool):
        clear = (int(clear_rx) << 1) | int(clear_tx)
        return self.send_api_cmd([SI4463API.FIFO_INFO, clear], 0)

    def get_state(self):
        return int.from_bytes(self.send_api_cmd([SI4463API.REQUEST_DEVICE_STATE], 1), "big")

    def set_property(self, prop: int, value: int, prop_len: int = 1) -> int:
        set_prop_cmd = [SI4463API.SET_PROPERTY, (prop >> 8) & 0xFF, prop_len, prop, value]
        return self.send_api_cmd(set_prop_cmd, 0)

    def set_packet_length(self, pkt_len: int) -> int:
        len_h = (pkt_len >> 8) & 0xFF
        len_l = (pkt_len >> 0) & 0xFF
        #     print(f"{len_h} , {len_l}")
        set_pkt_len_cmd = [SI4463API.SET_PROPERTY, 0x12, 2, 0x11, len_h, len_l]
        return self.send_api_cmd(set_pkt_len_cmd, 0)

    def set_tx_power(self, power: int) -> int:
        set_tx_pwr_cmd = [SI4463API.SET_PROPERTY, POWER_LEVEL_PROP >> 8, 1, power, 0x00]
        return self.send_api_cmd(set_tx_pwr_cmd, 0)

    def write_tx(self, data: list) -> int:
        count = min(len(data), TX_WRITE_BYTE_LIMIT)
        #         print(count)
        if count > 0:
            self.begin_comm()
            self._spi.write(bytearray([SI4463API.WRITE_TX_FIFO]))
            #     self._spi.write(bytearray(len(data)))
            self._spi.write(bytearray(data[:count]))
            self.end_comm()
        return count

    def set_state(self, new_state: int) -> int:
        return self.send_api_cmd([SI4463API.CHANGE_STATE, new_state], 0)

    def read_rx_fifo(self, rx_len: int) -> bytes:
        self.begin_comm()
        self._spi.write(bytearray([SI4463API.READ_RX_FIFO]))
        rx_data = self._spi.read(rx_len, 0xFF)
        self.end_comm()
        return rx_data

    # def read_rx(pkt_len):
    #     rx_data = read_rx_fifo(pkt_len)
    #     return rx_data

    def start_rx(self) -> int:
        self.clear_fifo(True, False)
        self.clear_int_status(clear_chip=0xff, clear_modem=0xff)
        self.set_packet_length(RADIO_RX_MAX_PACKET_LENGTH)
        # TODO: Rewrite without "magic" numbers and add parameters for start_rx command
        start_rx_cmd = [SI4463API.START_RX, 0x00, 0x00, 0x00, 0x00, 0, 3, 3]
        return self.send_api_cmd(start_rx_cmd, 0)

    def set_ready_state(self) -> int:
        set_rdy_cmd = [SI4463API.CHANGE_STATE, RADIO_STATE_READY]
        return self.send_api_cmd(set_rdy_cmd, 0)

    def handle_interrupt_status(self, stat: int) -> None:
        # TODO: add return statement for different flag status matches
        #         print(f"Info : {stat=}")

        if stat & TX_FIFO_ALMOST_EMPTY:
            pass
        #             print("TX FIFO ALMOST DONE")

        #

        if stat & PACKET_SENT_PEND:
            pass
        #             print("PACKET_SEND_PEND, ending transfer")

        if stat & SYNC_DETECT:
            print("SYNC")

        if stat & CMD_ERROR_PEND:
            print("CMD ERR PEND")

        if stat & FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND:
            print("FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND")
        #             self.set_ready_state()
        #             self.clear_fifo(clear_rx=True, clear_tx=True)
        if (stat & RX_FIFO_ALMOST_FULL):
            print("RX ALMOST FULL")
            self.rx_buff += self.read_rx_fifo(RADIO_FIFO_THRESHOLD)
        if (stat & PACKET_RX_PEND):
            print("PACKET_RX_PEND")
            self.obtain_packet()
        if (stat & PACKET_RX):
            print("PACKET RX")

    #             if not self.rx_blocked:
    #                 print("HANDLE PACKET_RX")
    #                 self.rx_tsf.set()
    #             else:
    #                 print("Sneaky handle")

    def obtain_packet(self):
        print("buffer before")
        print(self.rx_buff)
        pkt_info_resp = self.get_packet_info()
        print(f"pkt_info_resp : {list(pkt_info_resp)}")
        pkt_info_len = int.from_bytes(pkt_info_resp, "big")
        if pkt_info_len > len(self.rx_buff):
            attendant = self.read_rx_fifo(pkt_info_len - len(self.rx_buff))
            #             print(f"{attendant=}")
            self.rx_buff += attendant
        #         counter += 1
        # print(f"{list(rx_packet)=} with {len(rx_packet)=}")
        self.clear_int_status()
        print("buffer after : ")
        print(self.rx_buff)
        self.rx_tsf.set()

    def clear_rx_buff(self):
        self.rx_buff = bytes()

    def get_rx_packet(self):
        return self.rx_buff

    def rx_packet(self, wait_timeout: int, rx_timeout: int) -> bytes:
        # start_time = time.time()
        print("RX PACKET READ")
        counter = 0

        state = self.get_state()
        print(state)
        rx_packet = bytes()

        #         if state != RADIO_STATE_RX:
        #             print("setting rx state")
        #             self.start_rx()
        # TODO: add timeout checking
        print("yolo")
        #         irq_state = disable_irq()
        self.rx_blocked = True
        start_time = time.time()
        while True:
            int_read = self.read_int_status()
            status = int.from_bytes(int_read, "little")
            #             print(status)
            #         print(status)
            #             self.handle_interrupt_status(status)

            # state = handle_int_status(status)
            #         print(f"INT STATUS : {list(status)=}")
            #         counter +=1
            if status & RX_FIFO_ALMOST_FULL:
                # #             utime.sleep_ms(10)
                #             pkt_info_resp = send_api_cmd([0x16], 2)
                #             utime.sleep_ms(5)
                rx_packet += self.read_rx_fifo(RADIO_FIFO_THRESHOLD)
                print("RX_FIFO_ALMOST_FULL")

            if status & PACKET_RX:
                print("PACKET_RX_PEND")
                break
            if (start_time - time.time() > 5):
                print("timeout")
                break

        pkt_info_resp = self.get_packet_info()
        #     print(f"pkt_info_resp : {list(pkt_info_resp)}")
        pkt_info_len = int.from_bytes(pkt_info_resp, "big")
        if pkt_info_len > len(rx_packet):
            #             print(f"{pkt_info_len=} vs {len(rx_packet)}")
            attendant = self.read_rx_fifo(pkt_info_len - len(rx_packet))
            #             print(f"{attendant=}")
            rx_packet += attendant
        #         counter += 1
        # print(f"{list(rx_packet)=} with {len(rx_packet)=}")
        self.clear_int_status()
        sleep_ms(1000)
        #         enable_irq(irq_state)
        self.rx_blocked = False
        return rx_packet

    async def send_tx_packet(self, packet: list, channel: int) -> int:
        #         print(f"{self.get_state()=}")
        #         print("Packet transfer start")
        start = time.ticks_ms()
        self._irq_pin.irq(handler=self.handle_TX_ISR)
        #         print(f"{self.get_state()=}")
        if self.get_state() != SCI_READY_SATE:
            print("Bad state")
            return RET_DEVICE_NOT_READY
        self.clear_fifo(clear_rx=False, clear_tx=True)

        self.clear_int_status(clear_chip=0xff)
        self.set_packet_length(len(packet))

        print(f"Packet len ={len(packet)}")

        self.write_tx(self.to_be_bytes(len(packet)))

        count = self.write_tx(packet)
        packet = packet[count:]
        # TODO add try except block for bad send_api_cmd result case

        self.send_api_cmd([0x31, 0x00, 0x30, 0, 0], 0)

        while True:
            await self.tx_tsf.wait()
            int_read = self.read_int_status()
            status = int.from_bytes(int_read, "little")
            self.handle_interrupt_status(status)

            if status & TX_FIFO_ALMOST_EMPTY:
                count = self.write_tx(packet)
                packet = packet[count:]

            if status & PACKET_SENT_PEND:
                break

        #     print(get_state())
        self.set_packet_length(128)
        self.start_rx()
        self._irq_pin.irq(trigger=Pin.IRQ_FALLING, handler=self.handle_radio_ISR)
        end = time.ticks_ms()
        #         print("Time : {} ms".format(end-start))
        return RET_SUCCESS

    @staticmethod
    def to_uint8(val) -> None:
        return val % UINT8_MAX

    @staticmethod
    def to_fancy_hex(byte_arr: bytes) -> str:
        return ''.join(['0x{:02x} '.format(byte) for byte in byte_arr])

    @staticmethod
    def to_be_bytes(val: int) -> list[int]:
        h_b = (val >> 8) & 0xFF
        l_b = (val >> 0) & 0xFF
        return [h_b, l_b]
