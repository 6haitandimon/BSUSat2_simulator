UINT8_MAX = 256
UINT32_MAX = 4_294_967_296

def to_uint8(val):
    return val % UINT8_MAX

def to_uint32(val):
    return val % UINT32_MAX

class LFSRScrambler():
    @staticmethod
    def encode(data):
        reg = 0xAAAAAAAA
        res = list()
        for data_byte in data:
            data_byte ^= ((reg >> 4) ^ (reg >> 9))
            data_byte = to_uint8(data_byte)

            reg = (reg << 8) | data_byte
            reg = to_uint32(reg)
            res.append(data_byte)
        return res

    @staticmethod
    def decode(data):
        reg = 0xAAAAAAAA
        res = list()
        for data_byte in data:
            decoded_byte = to_uint8(data_byte ^ (reg >> 4) ^ (reg >> 9))
            reg = to_uint32((reg << 8) | data_byte)
            res.append(decoded_byte)
        return res



# def _show(msg):
#     scrm = g3ruh_scramble(msg)
#     descrm = g3ruh_descramble(scrm)
#     print('\nOriginal:\n' + msg)
#     print('Scrambled:\n' + scrm)
#     ##    print(g3ruh_descrambler(msg))
#     print('Descrambled:\n' + descrm)
#     ##    print(g3ruh_scrambler(g3ruh_descrambler(msg)))
#     print('\nIs descrambled = original? - ' + ('No.', 'Yes.')[descrm == msg])

def test_scrambler():
    # test_msg = '10110101011111010111101011110000111110101110001101011'
    # _show(test_msg)
    init_data = [0, 1, 2, 3, 0xff, 0xfe, 17, 19]
    encoded_data = LFSRScrambler.encode(init_data)
    print(encoded_data)
    decoded_data = LFSRScrambler.decode(encoded_data)
    print(decoded_data)
    print("Test passed" if init_data==decoded_data else "Test failed")
    # test_msg = list()
    # test_msg[:0] = '10110101011111010111101011110000111110101110001101011'
    # print(len(test_msg))

    # print(test_msg)
if __name__ == "__main__":
    test_scrambler()
