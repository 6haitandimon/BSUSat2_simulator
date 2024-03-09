import gc
from math import ceil

ONES_CAP = const(0x5)
HDLC_FLAG = const(0b_0111_1110)
FIVE_BITS = const(0b_0001_1111)
SIX_BITS = const(0b_0011_1111)
STUFFING = const(0b_0011_1110)
SEVEN_BITS = const(0b_0111_1111)

class HDLCEncoder:

    REVERSE_LOOKUP = [0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, ]
    


    _preamble_count = 7
    _trailer_count = 3


    # Decoder states
    NoSync = 0
    Sync = 1
    Recv = 2
    End = 3
    Error = 4
    NoData = 5

    @staticmethod
    def reverse_bits(n):
        return (HDLCEncoder.REVERSE_LOOKUP[(n & 0xF)] << 4) | (HDLCEncoder.REVERSE_LOOKUP[(n >> 4) & 0xFFFFFFFFFFFFFFFF])

    @staticmethod
    def process_viper_result(viper_res : bytearray) -> list:
        res = []
        for byte in viper_res:
            res.append(byte)
        
        trail_zeros_terminated = False
        while not trail_zeros_terminated:
            val = res.pop()
            if len(viper_res) <= 0 or val != 0:
                res.append(val)
                trail_zeros_terminated = True
        
        return res
    
    @staticmethod
    def encode(data: list, preamble_count: int = 7, trailer_count: int = 3) -> list:
        gc.collect()
        
        input_arr = bytearray(data)
        input_size = len(input_arr)
        output_max_size = ceil(input_size*1.2) + preamble_count + trailer_count
        output_arr = bytearray(output_max_size)
        HDLCEncoder._preamble_count = preamble_count
        HDLCEncoder._trailer_count = trailer_count
        
        HDLCEncoder.stuff_bits_with_viper(input_arr, input_size, output_arr, output_max_size)
        res = HDLCEncoder.process_viper_result(output_arr)
        
        del output_arr
        gc.collect()
        
        return res

    @staticmethod            
    @micropython.viper
    def stuff_bits_with_viper(input_arr, input_size: int, output_arr, output_size: int):
        
        data = ptr8(input_arr)
        output = ptr8(output_arr)
        
        reg : uint = 0
        bit_count : uint = 0
        one_count : uint = 0
        mask : uint = 0xFF
        val = 0
        preamble_count : int = int(HDLCEncoder._preamble_count)
        trailer_count : int = int(HDLCEncoder._trailer_count)
        
        for i in range(preamble_count):
            reg <<= 8
            reg |= HDLC_FLAG
            val = reg >> bit_count
            val = val & mask
            output[i] = val
        
        last_idx = preamble_count
        val = 0
    #     
        #main cycle
        for byte_idx in range(input_size):
            for bit_index in range(8):
                
                a = data[byte_idx] >> bit_index
                bit = a & 1

                reg <<= 1
                if bit == 1:
                    reg |= bit
                    one_count += 1
                elif bit == 0:
                    one_count = 0
                bit_count += 1
                
                if one_count == ONES_CAP:
                    reg <<= 1
                    bit_count += 1
                    one_count = 0
            
            if bit_count >= 8:
                bit_count -= 8
            elif bit_count > 0:
                reg <<= 8 - bit_count
                bit_count = 0
                
            val = reg >> bit_count
            val = val & mask
            output[last_idx+byte_idx] = val
        
        last_idx = last_idx + input_size   
        #trailer
        for i in range(trailer_count):
            reg <<= 8
            reg |= HDLC_FLAG
            bit_count += 8
            if bit_count >= 8:
                bit_count -=8
            val : uint = reg >> bit_count
            val = val & mask
            output[last_idx+i] = val
            
    @staticmethod
    def decode(data):
        res = []
        reg = 0
        bits = 0
        current = 0
        state = HDLCEncoder.NoSync
        for byte in data:
            for i in reversed(range(8)):
                bit = (byte >> i) & 1
                current <<= 1
                current &= 0xFF
                current |= bit
                if state == HDLCEncoder.Recv or state == HDLCEncoder.Sync:
                    reg <<= 1
                    reg &= 0xFFFFFFFF
                    reg |= bit
                    bits += 1

                    # check bit
                if current == HDLC_FLAG:
                    if state == HDLCEncoder.Recv:
                        if bits >= 8:
                            reg >>= 8
                            reg &= 0xFFFFFFFF
                            bits -= 8
                        if bits % 8 == 0:
                            state = HDLCEncoder.End
                        else:
                            print("leftover bits : {}".format(bits))
                            state = HDLCEncoder.Error
                    elif state == HDLCEncoder.NoSync or state == HDLCEncoder.Sync:
                        reg = 0
                        bits = 0
                        state = HDLCEncoder.Sync
                    elif state == HDLCEncoder.End or state == HDLCEncoder.Error or state == HDLCEncoder.NoData:
                        # print("Specific state consumed: {}".format(state))
                        pass
                elif current & SEVEN_BITS == SEVEN_BITS:
                    if state == HDLCEncoder.Recv:
                        print("Warn : seven bits")
                        state = HDLCEncoder.Error
                elif current & SIX_BITS == STUFFING:
                    if state == HDLCEncoder.Recv or state == HDLCEncoder.Sync:
                        #assert!(self.bits >= 1)
                        assert(bits >= 1)
                        reg >>= 1
                        reg &= 0xFFFFFFFF
                        bits -= 1
                if state == HDLCEncoder.Sync and bits >= 8:
                    state = HDLCEncoder.Recv
            if bits >= 8:
                bits -= 8
                res.append(HDLCEncoder.reverse_bits((reg >> bits) & 0xFF))
        return res

