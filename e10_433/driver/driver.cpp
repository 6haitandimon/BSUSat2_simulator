#include "driver.h"

ReturnCodes SI4463::init() {

    return RET_FAIL;
}

void SI4463::si446x_chip_init() {

}

etl::list<uint8_t, 256> SI4463::send_api_cmd(etl::vector<uint8_t, 16> txdata, int resp_len) {
    return etl::list<uint8_t, 256>();
}

etl::list<uint8_t, 256> SI4463::get_packet_info() {
    return etl::list<uint8_t, 256>();
}

etl::list<uint8_t, 256> SI4463::read_rx_fifo(int rx_len) {
    return etl::list<uint8_t, 256>();
}

etl::list<uint8_t, 256> SI4463::get_rx_packet() {
    return etl::list<uint8_t, 256>();
}

etl::list<uint8_t, 256> SI4463::rx_packet(int wait_timeout, int rx_timeout) {
    return etl::list<uint8_t, 256>();
}

void SI4463::begin_comm() {

}

void SI4463::end_comm() {

}

bool SI4463::is_send_cmd_allowed() {
    return false;
}

bool SI4463::wait_for_cts() {
    return false;
}

int SI4463::clear_int_status(int clear_ph, int clear_modem, int clear_chip) {
    return 0;
}

int SI4463::read_int_status() {
    return 0;
}

int SI4463::write_config() {
    return 0;
}

void SI4463::clear_fifo(bool clear_rx, bool clear_tx) {

}

int SI4463::get_state() {
    return 0;
}

int SI4463::set_property(int prop, int value, int prop_len) {
    return 0;
}

int SI4463::set_packet_length(int pkt_len) {
    return 0;
}

int SI4463::set_tx_power(int power) {
    return 0;
}

int SI4463::set_state(int new_state) {
    return 0;
}

void SI4463::handle_interrupt_status(int stat) {

}

void SI4463::obtain_packet() {

}

void SI4463::clear_rx_buff() {

}
