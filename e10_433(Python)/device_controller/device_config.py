from micropython import const
from machine import SPI

# Singleton only

DEFAULT_FILE_NAME = "device_config.cfg"


class ErrorTable:
    SAT_OK = 0,
    SAT_ERROR = -1,
    SAT_BAD_STATE = -2,
    SAT_WRONG_INPUT = -3


class RadioHardwareConfig:
    SPI_RADIO_HARDWARE_CHANNEL = 1
    SPI_RADIO_DEFAULT_BAUDRATE = 5000000
    SPI_RADIO_FIRSTBIT = SPI.MSB
    SPI_RADIO_MISO_PIN = 12
    SPI_RADIO_CS_PIN = 13
    SPI_RADIO_SCK_PIN = 14
    SPI_RADIO_MOSI_PIN = 15

    RADIO_SDN_PIN = 3
    GPIO1_PIN = 5

    DEFAULT_RADIO_IRQ_PIN = 7
    RADIO_MODULE_CAN_ADDR = 0x01


class InternalSPIConfig:
    SPI_DEFAULT_FIRSTBIT = SPI.MSB
    SPI_DEFAULT_POLARITY = const(0)
    SPI_DEFAULT_PHASE = const(0)

    SPI_PICO_HARDWARE_CHANNEL = 0
    SPI_PICO_MOSI_PIN = 19
    SPI_PICO_SCK_PIN = 18
    SPI_PICO_CS_PIN = 17
    SPI_PICO_MISO_PIN = 16
    CAN_PICO_IRQ_PIN = 22


class SlotCommunicationProtocol:
    # Keys
    SLOT_COMMAND_KEY = "command"
    SLOT_ADDRESS_KEY = "address"
    SLOT_PARAMETERS_MASK_KEY = "command.mask"
    SLOT_COMMAND_PARAMETER_KEY = "param"
    SLOT_PAGE_INDEX_KEY = "page.index"
    SLOT_PAGE_DATA_KEY = "page.data"

    SLOT_TASK_ACTION = "action"
    SLOT_TASK_PARAMETERS = "task_params"
    SLOT_INTERNAL_DELAY = "dT"
    SLOT_TASK_REPEAT_COUNT = "rps"
    SLOT_TASK_TIMEOUT_VALUE = "timeout"
    SLOT_TASK_START_TIMEOUT = "T0"

    SLOT_RESPONSE_KEY = "response"
    SLOT_TELEMETRY_KEY = "telemetry"
    STATION_TIMESTAMP_KEY = "TS"

    # Available values
    SET_SIGNALIZATION_STATE = "set.signal"
    GET_SIGNALIZATION_STATE = "get.signal"
    SEND_SIGNALIZATION_STATE = "send.signal"

    GET_TELEMETRY_CMD_VALUE = "get.telemetry"
    SEND_TELEMETRY_CMD_VALUE = "send.telemetry"

    SET_STATE_CMD_VALUE = "set.state"
    GET_STATE_CMD_VALUE = "get.state"
    SEND_STATE_CMD_VALUE = "send.state"

    ADD_TASK_CMD_VALUE = "add.task"
    EXECUTE_CMD_VALUE = "exec"
    STATE_VALUE = "state"

    SEND_FILE_TRANSFER_INIT_VALUE = "file.transfer.init"
    SEND_FILE_PAGE_VALUE = "send.page"
    SEND_FILE_TRANSFER_END_VALUE = "file.transfer.end"


class SlotCanAddressTable:
    RADIO_MODULE_SLOT_ADDRESS = 0x01
    MAGNETOMETER_MODULE_SLOT_ADDRESS = 0x02
    GPS_MODULE_SLOT_ADDRESS = 0x03
    CAN_SEND_TEST_MODULE_ADDRESS = 0x09


class DeviceConfig:
    FIRMWARE_VER = "0.1"
    reset_count = 0
    param_1 = const(2)
    param_3 = const(3)
    state = tuple()
    RADIO_MODULE_DEFAULT_AX25_ADDRESS = "PICOS"
    STATION_EMITATOR_DEFAULT_ADDRESS = "PICORW"
    INTERNAL_SLOT_ADDRESS = 0x01
    INTERNAL_RADIO_AVAILABLE = const(0x1)

    # def parse_config(self, config : tuple):
    #     for key, value in config:
    #         state[key] = value

    # def load_config_from_file(self, cfg_file_path = DEFAULT_FILE_NAME):
    #     try:
    #         cfg_file = open(cfg_file_path, "r")
    #         # self.parse_config(umsgpack.loads(cfg_file.read()))
    #         cfg_file.close()
    #         # return ERROR.ERROR_OK
    #         return 0
    #     except FileNotFoundError:
    #         print("No such configuration file")
