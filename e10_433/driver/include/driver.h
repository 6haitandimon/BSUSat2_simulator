#pragma once
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "driver_config.h"
#include "etl/list.h"
//#include "etl/vector.h"

enum Commands : uint8_t {
    NOP = 0x00,
    PART_INFO = 0x01,
    FUNC_INFO = 0x10,
    SET_PROPERTY = 0x11,
    GET_PROPERTY = 0x12,
    GPIO_PIN_CFG = 0x13,
    FIFO_INFO = 0x15,
    PACKET_INFO = 0x16,
    IRCAL = 0x17,
    PROTOCOL_CFG = 0x18,
    GET_INT_STATUS = 0x20,
    GET_PH_STATUS = 0x21,
    GET_MODEM_STATUS = 0x22,
    GET_CHIP_STATUS = 0x23,
    START_TX = 0x31,
    START_RX = 0x32,
    REQUEST_DEVICE_STATE = 0x33,
    CHANGE_STATE = 0x34,
    READ_CMD_BUFF = 0x44,
    WRITE_TX_FIFO = 0x66,
    READ_RX_FIFO = 0x77
};

const uint8_t TX_WRITE_BYTE_LIMIT = 24;
const uint8_t SI446X_CMD_READ_CMD_BUFF = 0x44;
const uint16_t SI446X_PKT_FIELD_2_LENGTH_LOW = ((0x12 << 8) | 0x12);

enum ReturnCodes : int {
    RET_FAIL = -1,
    RET_OP_TIMEOUT = -2,
    RET_SUCCESS = 0,
    RET_WRONG_CMD = 1,
    RET_NO_CTS = 2,
    RET_DEVICE_NOT_READY = 3
};

enum RadioStates : uint8_t {
    SCI_READY_SATE = 3,
    RADIO_STATE_RX = 8,
    RADIO_STATE_READY = 3
};

const uint16_t POWER_LEVEL_PROP = ((0x22 << 8) | 0x01);

const uint8_t SI446X_CMD_FIFO_INFO = 0x15;
const uint8_t SI446X_FIFO_CLEAR_RX = 0x02;
const uint8_t SI446X_FIFO_CLEAR_TX = 0x01;
const uint8_t RADIO_FIFO_THRESHOLD = 0x30;
const uint8_t DEFAULT_CLEAR_PH = 0x00;
const uint8_t DEFAULT_CLEAR_MODEM = 0x00;
const uint8_t DEFAULT_CLEAR_CHIP = 0x00;
const uint16_t RADIO_RX_MAX_PACKET_LENGTH = 512;

enum InterruptFlags : uint64_t {
    PACKET_RX = 0x10000000,
    SYNC_DETECT = 0x10000000000,
    RX_FIFO_ALMOST_FULL = 0x1000000,
    PACKET_RX_PEND = 0x100000,
    FIFO_UNDERFLOW_OVERFLOW_ERROR_PEND = 0x20000000000000,
    TX_FIFO_ALMOST_EMPTY = 0x2000000,
    PACKET_SENT_PEND = 0x200000,
    CMD_ERROR_PEND = 0x8000000000000
};

class SI4463 {
private:
    spi_inst_t* _spi;
    int _sdnPin;
    int _sckPin;
    int _misoPin;
    int _mosiPin;
    int _csPin;
    int _irqPin;
    bool rxBlocked;
    bool txBlockAvailable;
public:
    SI4463(spi_inst_t* spi, int sdnPin, int sckPin, int misoPin, int mosiPin, int csPin, int irqPin = -1) :
            _spi(spi), _sdnPin(sdnPin), _sckPin(sckPin), _misoPin(misoPin), _mosiPin(mosiPin), _csPin(csPin), _irqPin(irqPin),
            rxBlocked(false), txBlockAvailable(false) {};

    ReturnCodes init();
    void si446x_chip_init();
    etl::list<uint8_t, 256> send_api_cmd(etl::vector<uint8_t, 16> txdata, int resp_len);
    etl::list<uint8_t, 256> get_packet_info();
    etl::list<uint8_t, 256> read_rx_fifo(int rx_len);
    etl::list<uint8_t, 256> get_rx_packet();
    etl::list<uint8_t, 256> rx_packet(int wait_timeout, int rx_timeout);
    void begin_comm();
    void end_comm();
    bool is_send_cmd_allowed();
    bool wait_for_cts();
    int clear_int_status(int clear_ph = DEFAULT_CLEAR_PH, int clear_modem = DEFAULT_CLEAR_MODEM, int clear_chip = DEFAULT_CLEAR_CHIP);
    int read_int_status();
    int write_config();
    void clear_fifo(bool clear_rx, bool clear_tx);
    int get_state();
    int set_property(int prop, int value, int prop_len = 1);
    int set_packet_length(int pkt_len);
    int set_tx_power(int power);
    int set_state(int new_state);
    void handle_interrupt_status(int stat);
    void obtain_packet();
    void clear_rx_buff();
};