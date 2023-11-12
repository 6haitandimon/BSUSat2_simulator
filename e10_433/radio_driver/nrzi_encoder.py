NRZI_ENCODE_TABLE = [0xa, 0xb, 0x9, 0x8, 0xd, 0xc, 0xe, 0xf, 0x5, 0x4, 0x6, 0x7, 0x2, 0x3, 0x1, 0x0]


def hex_to_bin(hl: list) -> str:
    bin_conv = list()
    for hex_val in hl:
        binary = bin(hex_val)[2:].zfill(8)[::1]
        # print(binary)
        bin_conv.append(binary)

    return ''.join(bin_conv)


class NRZIEncoder():

    # def __init__(self):
    #     self.state = 1

    @staticmethod
    def encode_via_simple_stuffing(msg: str) -> list:
        last_bit = 0
        res = list()
        binary_msg = hex_to_bin(msg)
        print("before nrzi")
        print(binary_msg)
        # print(f"Binary msg from hex->bin conversion: {binary_msg}")
        # binary_msg = '0100011001000'
        for bit_literal in binary_msg:
            input_bit = int(bit_literal)
            last_bit = (input_bit ^ last_bit) & 1
            out_bit = last_bit
            res.append(out_bit)
        return res

    @staticmethod
    def encode(data: list) -> list:
        state = 1
        res = []
        for byte in data:
            hi = NRZI_ENCODE_TABLE[(byte >> 4) & 0xf]
            lo = NRZI_ENCODE_TABLE[byte & 0xf]
            if state != 0:
                hi ^= 0xf
            if hi & 1 != 0:
                lo ^= 0xf
            state = lo & 1
            res.append((hi << 4) | lo)
        return res

    @staticmethod
    def decode(data: list) -> list:
        state = 1
        res = []
        for byte in data:
            decoded_val = ~(byte ^ ((byte >> 1) | (state << 7))) & 0xFF
            state = byte & 1
            res.append(decoded_val)
        return res


if __name__ == "__main__":
    test_msg = [0, 1, 2, 3, 0xff, 0xfe, 17, 19]
    print(test_msg)
    nrzi_applied = NRZIEncoder.encode(test_msg)
    decoded_message = NRZIEncoder.decode(nrzi_applied)
    print(decoded_message)
    print("Test passed" if decoded_message == test_msg else "Test failed")
    # convereted_int_resp = int.from_bytes(test, byteorder="little")
    # if convereted_int_resp & 0x1000000:
    #     print("Yolo")
    # print(hex_to_bin(test))
    # converted_sample = hex_to_bin(test)
    # zero_counter = 0
    # one_counter = 0
    # for i in converted_sample:
    #     if i == '0':
    #         zero_counter += 1
    #     else:
    #         one_counter += 1
    # print(f"zero_counter : {zero_counter} and one_counter : {one_counter}")
