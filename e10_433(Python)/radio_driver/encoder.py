from radio_driver.AX25header import create_AX25_address, create_AX25_packet, AX25Packet
from radio_driver.g3ruh_scrambler import LFSRScrambler
from radio_driver.nrzi_encoder import NRZIEncoder
from radio_driver.hdlc_encoder import HDLCEncoder
from radio_driver.crc import calculate_crc
from time import ticks_ms
from umsgpack import dumps


def decode_radio_message(encoded_data) -> list:
    return HDLCEncoder.decode(
        LFSRScrambler.decode(
            NRZIEncoder.decode(
                encoded_data
            )
        )
    )


def encode_radio_message(data) -> list:
    crc_bytes = calculate_crc(data)
    data.extend(crc_bytes)
    return NRZIEncoder.encode(
        LFSRScrambler.encode(
            HDLCEncoder.encode(
                data
            )
        )
    )
