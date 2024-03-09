# Special address description flags for the CAN_ID
CAN_EFF_FLAG = 0x80000000  # EFF/SFF is set in the MSB
CAN_RTR_FLAG = 0x40000000  # remote transmission request
CAN_ERR_FLAG = 0x20000000  # error message frame
CAN_LAST_CHUNK_FLAG = 0x80000

# Valid bits in CAN ID for frame formats
CAN_SFF_MASK = 0x000007FF  # standard frame format (SFF)
CAN_EFF_MASK = 0x1FFFFFFF  # extended frame format (EFF)
CAN_ERR_MASK = 0x1FFFFFFF  # omit EFF, RTR, ERR flags

CAN_SFF_ID_BITS = 11
CAN_EFF_ID_BITS = 29

# CAN payload length and DLC definitions according to ISO 11898-1
CAN_MAX_DLC = 8
CAN_MAX_DLEN = 8

# CAN ID length
CAN_IDLEN = 4

SRC_ADDR_START_IDX = 0
DST_ADDR_START_IDX = 4
CMD_ID_START_IDX = 20
CHUNK_ID_START_IDX = 8
MSG_PRIORITY_START_IDX = 26

class CANFrame:
    def __init__(self, can_id: int, data: bytes = b"") -> None:
        #
        # Controller Area Network Identifier structure
        #
        # bit 0-28 : CAN identifier (11/29 bit)
        # bit 29   : error message frame flag (0 = data frame, 1 = error message)
        # bit 30   : remote transmission request flag (1 = rtr frame)
        # bit 31   : frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
        #
        # 32 bit CAN ID + EFF/RTR/ERR flags
        #
        self.can_id = can_id  # type: int
        self.data = data  # type: bytes

    @property
    def can_id(self) -> int:
        return self._can_id

    @property
    def dst_addr(self) -> int:
        return self._can_id >> DST_ADDR_START_IDX & 0x0F

    @property
    def src_addr(self) -> int:
        return self._can_id >> SRC_ADDR_START_IDX & 0x0F

    @property
    def cmd_idx(self) -> int:
        return self._can_id >> CMD_ID_START_IDX & 0x3F

    @property
    def chunk_idx(self) -> int:
        return self._can_id >> CHUNK_ID_START_IDX & 0x7FF

    @property
    def is_chunk_last(self) -> bool:
        return bool(self._can_id & CAN_LAST_CHUNK_FLAG)

    @property
    def msg_priority(self) -> int:
        return self._can_id >> MSG_PRIORITY_START_IDX & 0x07

    @can_id.setter
    def can_id(self, can_id: int) -> None:
        self._can_id = can_id
        self._arbitration_id = can_id & CAN_EFF_MASK  # type: int

    @property
    def data(self) -> bytes:
        return self._data

    @data.setter
    def data(self, data: bytes) -> None:
        self._data = b""  # type: bytes
        self._dlc = 0  # frame payload length in byte (0 .. CAN_MAX_DLEN)

        if not data:
            return

        if len(data) > CAN_MAX_DLEN:
            raise Exception("The CAN frame data length exceeds the maximum")

        self._data = data
        self._dlc = len(data)

    @property
    def arbitration_id(self) -> int:
        return self._arbitration_id

    @property
    def dlc(self) -> int:
        return self._dlc

    @property
    def is_extended_id(self) -> bool:
        return bool(self._can_id & CAN_EFF_FLAG)

    @property
    def is_remote_frame(self) -> bool:
        return bool(self._can_id & CAN_RTR_FLAG)

    @property
    def is_error_frame(self) -> bool:
        return bool(self._can_id & CAN_ERR_FLAG)

    @staticmethod
    def build_frame_id(cmd_id: int, priority: int = 0b001, chunk_id: int = 0x800, dst_addr: int = 0x5,
                       src_addr: int = 0x3):
        can_id = 0x80000000 | (priority << 26) | (cmd_id << 20) | (chunk_id << 8) | (dst_addr << 4) | src_addr
        return can_id

    def __str__(self) -> str:
        data = (
            "remote request"
            if self.is_remote_frame
            else " ".join("{:02X}".format(b) for b in self.data)
        )
        return "{: >8X}   [{}]  {}".format(self.arbitration_id, self.dlc, data)

