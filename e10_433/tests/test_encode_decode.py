from radio_driver.AX25header import create_AX25_address, create_AX25_packet, AX25Packet
from radio_driver.crc import calculate_crc
from radio_driver.encoder import decode_radio_message
from radio_driver.g3ruh_scrambler import LFSRScrambler
from radio_driver.hdlc_encoder import HDLCEncoder
from radio_driver.nrzi_encoder import NRZIEncoder
from time import ticks_ms
from umsgpack import dumps

AX_TEST_PACKET = [
    0x92, 0x88, 0x40, 0x40, 0x40, 0x40, 0xe0, 0xac, 0x96, 0x6e, 0x90, 0x88, 0x9a, 0x6d, 0x03,
    0xf0, 0x56, 0x4b, 0x37, 0x48, 0x44, 0x4d, 0x2d, 0x36, 0x20, 0x50, 0x61, 0x63, 0x6b, 0x65,
    0x74, 0x20, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x47, 0x61, 0x74, 0x65,
    0x77, 0x61, 0x79, 0x20, 0x51, 0x45, 0x33, 0x37, 0x50, 0x47,
]
AX_TEST_DST_ADDRESS = "ID"
AX_TEST_SRC_ADDRESS = "VK7HDM"
TEST_ENCODE_MESSAGE = {
    "command": "send.telemetry",
    "address": 0x01,
    "VSolar": 5.5,
    "VBat1": 4.25,
    "VBat2": 3.75,
    "VSys": 4.0,
    "ISys": 170,
    "Temp": 24
}


def tag_n_show_equality(expr: bool) -> None:
    if expr:
        print('\x1b[6;30;42m' + 'Check Success!' + '\x1b[0m')
    else:
        print('\x1b[3;30;41m' + 'Check Failed!' + '\x1b[0m')


def test_full_encoding_chain(show_encoders_outputs=False) -> None:
    dst_addr = create_AX25_address("MOON", 0, True)
    src_addr = create_AX25_address("BSU", 6, False)
    payload = [134, 162, 84, 83, 206, 98, 225, 18, 215, 163]
    packet = create_AX25_packet(src_addr, dst_addr, payload)

    # Can get via RUST tests
    expected_output = (
        [154, 158, 158, 156, 64, 64, 224, 132, 166, 170, 64, 64, 64, 109, 3, 240, 134, 162, 84, 83, 206, 98, 225, 18,
         215, 163],
        [126, 126, 126, 126, 126, 126, 126, 89, 121, 121, 57, 2, 2, 7, 33, 101, 85, 2, 2, 2, 182, 192, 15, 97, 69, 42,
         202, 115, 70, 135, 72, 235, 197, 126, 126, 126],
        [129, 131, 38, 141, 133, 224, 226, 167, 34, 88, 141, 166, 158, 189, 133, 227, 201, 207, 122, 18, 170, 227, 244,
         47, 253, 194, 232, 188, 185, 18, 133, 74, 211, 118, 32, 167],
        [171, 168, 145, 163, 172, 21, 233, 144, 150, 197, 163, 145, 190, 124, 83, 232, 36, 32, 249, 73, 153, 232, 13,
         96, 3, 214, 26, 125, 132, 182, 83, 38, 55, 14, 149, 144],
    )
    pkt_with_header = packet.encode()
    print("----------Payload with AX25 header----------")
    if show_encoders_outputs:
        print(pkt_with_header)
        print("expected:")
        print(expected_output[0])
    assert (pkt_with_header == expected_output[0])
    tag_n_show_equality(pkt_with_header == expected_output[0])

    print("----------HDLC bitstuffing----------")
    hdlc_wrapped_pkt = HDLCEncoder.encode(pkt_with_header)
    if show_encoders_outputs:
        print(hdlc_wrapped_pkt)
        print("expected:")
        print(expected_output[1])
    assert (hdlc_wrapped_pkt == expected_output[1])
    tag_n_show_equality(hdlc_wrapped_pkt == expected_output[1])

    print("----------G3RUH scramble----------")
    g3ruh_scrambled = LFSRScrambler.encode(hdlc_wrapped_pkt)
    if show_encoders_outputs:
        print(g3ruh_scrambled)
        print("expected:")
        print(expected_output[2])
    assert (g3ruh_scrambled == expected_output[2])
    tag_n_show_equality(g3ruh_scrambled == expected_output[2])

    print("----------NRZI----------")
    nrzi_applied = NRZIEncoder.encode(g3ruh_scrambled)
    if show_encoders_outputs:
        print(nrzi_applied)
        print("expected:")
        print(expected_output[3])
    assert (nrzi_applied == expected_output[3])
    tag_n_show_equality(nrzi_applied == expected_output[3])


def test_encode_AX25():
    # Encoding
    id_addr = create_AX25_address(AX_TEST_DST_ADDRESS, 0, True)
    vk7_addr = create_AX25_address(AX_TEST_SRC_ADDRESS, 6, False)
    payload = b"VK7HDM-6 Packet Internet Gateway QE37PG"
    packet = create_AX25_packet(vk7_addr, id_addr, payload)
    res = packet.encode()
    print(res)
    assert (res == AX_TEST_PACKET)
    print("AX25 Encode test passed")


def test_decode_AX25():
    # Decoding
    ax25_packet = AX25Packet.parse(AX_TEST_PACKET)

    assert (ax25_packet.dst_addr.get_call_sign() == AX_TEST_DST_ADDRESS and
            ax25_packet.src_addr.get_call_sign() == AX_TEST_SRC_ADDRESS,
            f"Decoded dst or source address are differs from initial addresses: \
            \nInitial\n{AX_TEST_DST_ADDRESS} {AX_TEST_SRC_ADDRESS} \
            \nDecoded\n{ax25_packet.dst_addr.get_call_sign()} {ax25_packet.src_addr.get_call_sign()}\n")

    assert (ax25_packet.payload == list(b"VK7HDM-6 Packet Internet Gateway QE37PG"),
            f"decoded payload is differ from initial payload: \
            \nInitial\n{AX_TEST_PACKET} \
            \nDecoded\n{ax25_packet.payload}\n")

    print("AX25 Parse test passed")


def test_encode_decode_timings(packet: AX25Packet = None) -> None:
    import time
    if packet is None:
        src_addr = create_AX25_address("BSUSat", 6, False)
        dst_addr = create_AX25_address("BSUGS", 0, True)
        packet = create_AX25_packet(src_addr, dst_addr, dumps(TEST_ENCODE_MESSAGE))
    print("AX25 packet: {}".format(packet))
    packet = packet.encode()
    crc_start = time.ticks_ms()
    crc_bytes = calculate_crc(packet)
    packet.extend(crc_bytes)
    crc_end = time.ticks_ms()
    print("Encoded packet: {}".format(packet))

    # Apply HDLC + LFSR + NRZI encodings
    encode_start = time.ticks_ms()
    encoded_data = HDLCEncoder.encode(packet)
    nrzi_end = time.ticks_ms()
    encoded_data = LFSRScrambler.encode(encoded_data)
    lfsrs_end = time.ticks_ms()
    encoded_data = NRZIEncoder.encode(encoded_data)
    encode_end = time.ticks_ms()
    print(f"""Encode time:
              CRC  : {crc_end - crc_start}
              HDLC : {nrzi_end - encode_start}
              LFSR : {lfsrs_end - nrzi_end}
              NRZI : {encode_end - lfsrs_end}
              FULL : {encode_end - encode_start + crc_end - crc_start}
            """)
    decode_start = time.ticks_ms()
    decoded_data = decode_radio_message(encoded_data)
    decode_end = time.ticks_ms()
    print(f"Decode time : {decode_end - decode_start}")

    print("Decoded packet: {}".format(decoded_data))

    endway_packet = AX25Packet.parse(decoded_data)
    print("AX25 Packet: {}".format(endway_packet))


if __name__ == "__main__":
    test_encode_decode_timings()

    test_full_encoding_chain()

[126, 126, 126, 126, 126, 193, 197, 150, 38, 30, 72, 37, 38, 134, 46, 134, 155, 131, 194, 22, 150, 198, 22, 150, 214, 246, 110, 4, 134, 118, 38, 4, 22, 150, 206, 4, 198, 134, 78, 78, 150, 134, 230, 166, 4, 78, 166, 134, 198, 22, 4, 46, 22, 166, 4, 46, 134, 110, 166, 78, 118, 4, 46, 22, 134, 46, 4, 206, 166, 166, 182, 206, 4, 54, 150, 214, 166, 4, 68, 134, 4, 74, 174, 206, 206, 150, 134, 118, 4, 14, 166, 134, 206, 134, 118, 46, 4, 198, 246, 46, 46, 134, 230, 166, 52, 4, 246, 118, 4, 134, 4, 206, 246, 182, 166, 238, 22, 134, 46, 4, 70, 150, 230, 230, 166, 78, 4, 206, 198, 134, 54, 166, 116, 68, 4, 18, 166, 4, 166, 118, 46, 166, 78, 206, 4, 134, 118, 38, 4, 246, 78, 38, 166, 78, 206, 4, 14, 134, 78, 46, 4, 246, 102, 4, 134, 4, 206, 174, 198, 214, 54, 150, 118, 230, 4, 14, 150, 230, 4, 238, 150, 46, 22, 4, 22, 246, 78, 206, 166, 78, 134, 38, 150, 206, 22, 4, 134, 118, 38, 4, 206, 246, 174, 78, 4, 198, 78, 166, 134, 182, 116, 197, 198, 182, 38, 149, 102, 150, 54, 166, 116, 206, 166, 118, 38, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126]
[126, 126, 126, 126, 126, 193, 197, 150, 38, 30, 72, 37, 38, 134, 46, 134, 155, 131, 194, 22, 150, 198, 22, 150, 214, 246, 110, 4, 134, 118, 38, 4, 22, 150, 206, 4, 198, 134, 78, 78, 150, 134, 230, 166, 4, 78, 166, 134, 198, 22, 4, 46, 22, 166, 4, 46, 134, 110, 166, 78, 118, 4, 46, 22, 134, 46, 4, 206, 166, 166, 182, 206, 4, 54, 150, 214, 166, 4, 68, 134, 4, 74, 174, 206, 206, 150, 134, 118, 4, 14, 166, 134, 206, 134, 118, 46, 4, 198, 246, 46, 46, 134, 230, 166, 52, 4, 246, 118, 4, 134, 4, 206, 246, 182, 166, 238, 22, 134, 46, 4, 70, 150, 230, 230, 166, 78, 4, 206, 198, 134, 54, 166, 116, 68, 4, 18, 166, 4, 166, 118, 46, 166, 78, 206, 4, 134, 118, 38, 4, 246, 78, 38, 166, 78, 206, 4, 14, 134, 78, 46, 4, 246, 102, 4, 134, 4, 206, 174, 198, 214, 54, 150, 118, 230, 4, 14, 150, 230, 4, 238, 150, 46, 22, 4, 22, 246, 78, 206, 166, 78, 134, 38, 150, 206, 22, 4, 134, 118, 38, 4, 206, 246, 174, 78, 4, 198, 78, 166, 134, 182, 116, 197, 198, 182, 38, 149, 102, 150, 54, 166, 116, 206, 166, 118, 38, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126]
