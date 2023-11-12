# from device_controller.device_config import ErrorTable

AX25_ADDR_LEN = 7
AX25_CALL_SIGN_LEN = 6
AX25_RESERVED_BITS = 0b0_11_0000_0
AX25_SSID_MASK = 0xf
AX25_CMD_MASK = 0x80
AX25_FINAL_MASK = 1
AX25_UI_FRAME_CONTROL = 0x03
AX25_UI_FRAME_PROTOCOL = 0xF0


class BadAX25AddressLengthException(Exception):
    def __init__(self, msg=""):
        super().__init__(msg)


class FailedAX25AddressParseException(Exception):
    def __init__(self, msg=""):
        super().__init__(msg)


class FailedAX25PacketParseException(Exception):
    def __init__(self, msg=""):
        super().__init__(msg)


class BrokenAX25AddressFieldException(Exception):
    def __init__(self, msg=""):
        super().__init__(msg)


class AX25Address:
    call_sign: str = ""
    flags: int = 0

    def __init__(self, call_sign_str, flags):
        self.call_sign = call_sign_str
        self.flags = flags

    @staticmethod
    def parse(data):
        if len(data) < AX25_ADDR_LEN:
            raise FailedAX25AddressParseException(f"Bad address length to parse : {len(data)}")
        call_sign = ""
        for i in range(AX25_CALL_SIGN_LEN):
            byte = data[i]
            if byte & AX25_FINAL_MASK == AX25_FINAL_MASK:
                raise FailedAX25AddressParseException(f"Address field is broken")
            call_sign += chr(data[i] >> 1)
        flags = [AX25_CALL_SIGN_LEN]
        return AX25Address(call_sign, flags)

    def encode(self, is_final):
        result = list()
        # print(f"len of call_sign is {len(self.call_sign)}")
        assert (len(self.call_sign) == AX25_CALL_SIGN_LEN)
        encoded_call_sign = self.call_sign.encode("UTF-8")
        for byte in encoded_call_sign:
            result.append(byte << 1)
        result.append(self.flags | int(is_final))
        return result

    def get_call_sign(self):
        return self.call_sign.strip()

    def __str__(self):
        return "{}".format(self.call_sign)


class AX25Packet:
    dst_addr = None
    src_addr = None
    payload = []

    def __init__(self, dst_addr, src_addr, payload):
        self.src_addr = src_addr
        self.dst_addr = dst_addr
        self.payload = payload

    @staticmethod
    def parse(data):

        addr_end = 0
        for i in range(len(data)):
            if data[i] & AX25_FINAL_MASK == AX25_FINAL_MASK:
                addr_end = i + 1
                break

        if AX25Packet.is_address_field_boundary_incorrect(addr_end):
            raise BrokenAX25AddressFieldException("Parse failed : broken address field boundaries")

        address_chunk = data[:addr_end]

        dst_addr = AX25Address.parse(address_chunk[0: AX25_ADDR_LEN])
        src_addr = AX25Address.parse(address_chunk[AX25_ADDR_LEN: addr_end])

        flags_end = addr_end + 2

        if flags_end > len(data):
            raise FailedAX25PacketParseException("Parse failed : truncated packet")
        payload = bytearray(data[flags_end:])
        return AX25Packet(dst_addr, src_addr, payload)

    @staticmethod
    def is_address_field_boundary_incorrect(addr_end):
        return addr_end == 0 or addr_end % AX25_ADDR_LEN != 0

    def encode(self):
        out = []
        encoded_dst_addr = self.dst_addr.encode(False)
        encoded_src_addr = self.src_addr.encode(True)
        out.extend(encoded_dst_addr)
        out.extend(encoded_src_addr)
        out.append(AX25_UI_FRAME_CONTROL)
        out.append(AX25_UI_FRAME_PROTOCOL)
        out.extend(self.payload)
        return out

    def __str__(self):
        return "SRC:{} DST:{} Payload:{}".format(self.src_addr, self.dst_addr, self.payload)


def create_AX25_address(cs_str, ssid, is_command):
    if len(cs_str) > AX25_CALL_SIGN_LEN:
        raise BadAX25AddressLengthException(
            f"Wrong AX25 Address len : {len(cs_str)}, expected {AX25_CALL_SIGN_LEN} or less")
    while not len(cs_str) == AX25_CALL_SIGN_LEN:
        cs_str += " "
    call_sign = cs_str
    cmd = AX25_CMD_MASK if is_command else 0
    flags = AX25_RESERVED_BITS | ((ssid & AX25_SSID_MASK) << 1) | cmd
    return AX25Address(call_sign, flags)


def create_AX25_packet(src_addr, dst_addr, payload):
    return AX25Packet(dst_addr, src_addr, payload)
