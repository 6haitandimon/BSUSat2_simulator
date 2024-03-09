#include <string.h>
#include "mcp2515.h"

static const struct TXBn_REGS TXB[N_TXBUFFERS] = {
    {MCP_TXB0CTRL, MCP_TXB0SIDH, MCP_TXB0DATA},
    {MCP_TXB1CTRL, MCP_TXB1SIDH, MCP_TXB1DATA},
    {MCP_TXB2CTRL, MCP_TXB2SIDH, MCP_TXB2DATA}
};

static const struct RXBn_REGS RXB[N_RXBUFFERS] = {
    {MCP_RXB0CTRL, MCP_RXB0SIDH, MCP_RXB0DATA, CANINTF_RX0IF},
    {MCP_RXB1CTRL, MCP_RXB1SIDH, MCP_RXB1DATA, CANINTF_RX1IF}
};


MCP2515 _MCP2515_init(MCP2515 mcp2515, spi_inst_t* spi, uint8_t cs_pin, uint8_t tx_pin, uint8_t rx_pin, uint8_t sck_pin, uint32_t spi_clock){
    mcp2515._SPI_CHANNEL = spi;
    mcp2515._SPI_CS_PIN = cs_pin;
    mcp2515._TX_PIN = tx_pin;
    mcp2515._RX_PIN = rx_pin;
    mcp2515._SCK_PIN = sck_pin;
    mcp2515._SPI_CLOCK = spi_clock;

    spi_init(mcp2515._SPI_CHANNEL, mcp2515._SPI_CLOCK);
    gpio_set_function(mcp2515._SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(mcp2515._TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(mcp2515._RX_PIN, GPIO_FUNC_SPI);
    spi_set_format(mcp2515._SPI_CHANNEL, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_init(mcp2515._SPI_CS_PIN);
    gpio_set_dir(mcp2515._SPI_CS_PIN, GPIO_OUT);

    private_stopSPI(&mcp2515);

    return mcp2515;
}

void private_startSPI(MCP2515* mcp2515){
    gpio_put(mcp2515->_SPI_CS_PIN, 0);
}

void private_stopSPI(MCP2515* mcp2515){
    gpio_put(mcp2515->_SPI_CS_PIN, 1);
}

ERROR reset(MCP2515* mcp2515){
    private_startSPI(mcp2515);

    uint8_t instruction = INSTRUCTION_RESET;
    spi_write_blocking(mcp2515->_SPI_CHANNEL, &instruction, 1);
    private_stopSPI(mcp2515);

    sleep_ms(10);

    uint8_t zeros[14];
    memset(zeros, 0, 14);

    private_setRegistersManual(mcp2515, MCP_TXB0CTRL, zeros, 14);
    private_setRegistersManual(mcp2515, MCP_TXB1CTRL, zeros, 14);
    private_setRegistersManual(mcp2515, MCP_TXB2CTRL, zeros, 14);

    private_setRegister(mcp2515, MCP_RXB0CTRL, 0);
    private_setRegister(mcp2515, MCP_RXB1CTRL, 0);

    private_setRegister(mcp2515, MCP_CANINTE, CANINTF_RX0IF | CANINTF_RX1IF | CANINTF_ERRIF | CANINTF_MERRF);

    private_modifyRegister(mcp2515, MCP_RXB0CTRL, RXBnCTRL_RXM_MASK | RXB0CTRL_BUKT | RXB0CTRL_FILHIT_MASK,
                   RXBnCTRL_RXM_STDEXT | RXB0CTRL_BUKT | RXB0CTRL_FILHIT);
    
    private_modifyRegister(mcp2515, MCP_RXB1CTRL, RXBnCTRL_RXM_MASK | RXB1CTRL_FILHIT_MASK,
                   RXBnCTRL_RXM_STDEXT | RXB1CTRL_FILHIT);

    
    RXF filters[] = {RXF0, RXF1, RXF2, RXF3, RXF4, RXF5};
    for (int i=0; i<6; i++) {
        bool ext = (i == 1);
        ERROR result = setFilter(mcp2515, filters[i], ext, 0);
        if (result != ERROR_OK) {
            return result;
        }
    }

    MASK masks[] = {MASK0, MASK1};
    for (int i=0; i<2; i++) {
        ERROR result = setFilterMask(mcp2515, masks[i], true, 0);
        if (result != ERROR_OK) {
            return result;
        }
    }

    return ERROR_OK;
    

}

uint8_t private_readRegister(MCP2515* mcp2515, const REGISTER reg)
{
    private_startSPI(mcp2515);

    uint8_t data[2] = {
        INSTRUCTION_READ,
        reg
    };

    spi_write_blocking(mcp2515->_SPI_CHANNEL, data, 2);

    uint8_t ret;
    spi_read_blocking(mcp2515->_SPI_CHANNEL, 0x00, &ret, 1);

    private_stopSPI(mcp2515);

    return ret;
}

uint8_t* private_readRegistersManual(MCP2515* mcp2515, const REGISTER reg, uint8_t *values, const uint8_t n)
{
    private_startSPI(mcp2515);

    uint8_t data[2] = {
        INSTRUCTION_READ,
        reg
    };
    spi_write_blocking(mcp2515->_SPI_CHANNEL, data, 2);

    spi_read_blocking(mcp2515->_SPI_CHANNEL, 0x00, values, n);

    private_stopSPI(mcp2515);

    return values;
}

void private_setRegister(MCP2515* mcp2515, const REGISTER reg, const uint8_t value)
{
    private_startSPI(mcp2515);

    uint8_t data[3] = {
        INSTRUCTION_WRITE,
        reg,
        value
    };
    spi_write_blocking(mcp2515->_SPI_CHANNEL, data, 3);

    private_stopSPI(mcp2515);
}

void private_setRegistersManual(MCP2515* mcp2515, const REGISTER reg, const uint8_t values[], const uint8_t n)
{
    private_startSPI(mcp2515);

    uint8_t data[2] = {
        INSTRUCTION_WRITE,
        reg
    };
    spi_write_blocking(mcp2515->_SPI_CHANNEL, data, 2);

    spi_write_blocking(mcp2515->_SPI_CHANNEL, values, n);

    private_stopSPI(mcp2515);
}

void private_modifyRegister(MCP2515* mcp2515, const REGISTER reg, const uint8_t mask, const uint8_t data)
{
    private_startSPI(mcp2515);

    uint8_t d[4] = {
        INSTRUCTION_BITMOD,
        reg,
        mask,
        data
    };

    spi_write_blocking(mcp2515->_SPI_CHANNEL, d, 4);

    private_stopSPI(mcp2515);
}

void private_prepareId(uint8_t *buffer, const bool ext, const uint32_t id)
{
    uint16_t canid = (uint16_t)(id & 0x0FFFF);

    if (ext) {
        buffer[MCP_EID0] = (uint8_t) (canid & 0xFF);
        buffer[MCP_EID8] = (uint8_t) (canid >> 8);
        canid = (uint16_t)(id >> 16);
        buffer[MCP_SIDL] = (uint8_t) (canid & 0x03);
        buffer[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
        buffer[MCP_SIDL] |= TXB_EXIDE_MASK;
        buffer[MCP_SIDH] = (uint8_t) (canid >> 5);
    } else {
        buffer[MCP_SIDH] = (uint8_t) (canid >> 3);
        buffer[MCP_SIDL] = (uint8_t) ((canid & 0x07 ) << 5);
        buffer[MCP_EID0] = 0;
        buffer[MCP_EID8] = 0;
    }
}

uint8_t getStatus(MCP2515* mcp2515)
{
    private_startSPI(mcp2515);

    uint8_t instruction = INSTRUCTION_READ_STATUS;
    spi_write_blocking(mcp2515->_SPI_CHANNEL, &instruction, 1);

    uint8_t ret;
    spi_read_blocking(mcp2515->_SPI_CHANNEL, 0x00, &ret, 1);

    private_stopSPI(mcp2515);

    return ret;
}

ERROR setConfigMode(MCP2515* mcp2515)
{
    return private_setMode(mcp2515, CANCTRL_REQOP_CONFIG);
}

ERROR setListenOnlyMode(MCP2515* mcp2515)
{
    return private_setMode(mcp2515, CANCTRL_REQOP_LISTENONLY);
}

ERROR setSleepMode(MCP2515* mcp2515)
{
    return private_setMode(mcp2515, CANCTRL_REQOP_SLEEP);
}

ERROR setLoopbackMode(MCP2515* mcp2515)
{
    return private_setMode(mcp2515, CANCTRL_REQOP_LOOPBACK);
}

ERROR setNormalMode(MCP2515* mcp2515)
{
    return private_setMode(mcp2515, CANCTRL_REQOP_NORMAL);
}

ERROR private_setMode(MCP2515* mcp2515, const CANCTRL_REQOP_MODE mode)
{
    private_modifyRegister(mcp2515, MCP_CANCTRL, CANCTRL_REQOP, mode);

    unsigned long endTime = to_ms_since_boot(get_absolute_time()) + 10;
    bool modeMatch = false;
    while (to_ms_since_boot(get_absolute_time()) < endTime) {
        uint8_t newmode = private_readRegister(mcp2515, MCP_CANSTAT);
        newmode &= CANSTAT_OPMOD;

        modeMatch = newmode == mode;

        if (modeMatch) {
            break;
        }
    }

    return modeMatch 
        ? ERROR_OK
        : ERROR_FAIL;
}

ERROR setBitrate(MCP2515* mcp2515, const CAN_SPEED canSpeed)
{
    return setBitrateManual(mcp2515, canSpeed, MCP_16MHZ);
}

ERROR setBitrateManual(MCP2515* mcp2515, const CAN_SPEED canSpeed, CAN_CLOCK canClock)
{
    ERROR error = setConfigMode(mcp2515);
    if (error != ERROR_OK) {
        return error;
    }

    uint8_t set, cfg1, cfg2, cfg3;
    set = 1;
    switch (canClock)
    {
        case (MCP_8MHZ):
        switch (canSpeed)
        {
            case (CAN_5KBPS):                                               //   5KBPS
            cfg1 = MCP_8MHz_5kBPS_CFG1;
            cfg2 = MCP_8MHz_5kBPS_CFG2;
            cfg3 = MCP_8MHz_5kBPS_CFG3;
            break;

            case (CAN_10KBPS):                                              //  10KBPS
            cfg1 = MCP_8MHz_10kBPS_CFG1;
            cfg2 = MCP_8MHz_10kBPS_CFG2;
            cfg3 = MCP_8MHz_10kBPS_CFG3;
            break;

            case (CAN_20KBPS):                                              //  20KBPS
            cfg1 = MCP_8MHz_20kBPS_CFG1;
            cfg2 = MCP_8MHz_20kBPS_CFG2;
            cfg3 = MCP_8MHz_20kBPS_CFG3;
            break;

            case (CAN_31K25BPS):                                            //  31.25KBPS
            cfg1 = MCP_8MHz_31k25BPS_CFG1;
            cfg2 = MCP_8MHz_31k25BPS_CFG2;
            cfg3 = MCP_8MHz_31k25BPS_CFG3;
            break;

            case (CAN_33KBPS):                                              //  33.333KBPS
            cfg1 = MCP_8MHz_33k3BPS_CFG1;
            cfg2 = MCP_8MHz_33k3BPS_CFG2;
            cfg3 = MCP_8MHz_33k3BPS_CFG3;
            break;

            case (CAN_40KBPS):                                              //  40Kbps
            cfg1 = MCP_8MHz_40kBPS_CFG1;
            cfg2 = MCP_8MHz_40kBPS_CFG2;
            cfg3 = MCP_8MHz_40kBPS_CFG3;
            break;

            case (CAN_50KBPS):                                              //  50Kbps
            cfg1 = MCP_8MHz_50kBPS_CFG1;
            cfg2 = MCP_8MHz_50kBPS_CFG2;
            cfg3 = MCP_8MHz_50kBPS_CFG3;
            break;

            case (CAN_80KBPS):                                              //  80Kbps
            cfg1 = MCP_8MHz_80kBPS_CFG1;
            cfg2 = MCP_8MHz_80kBPS_CFG2;
            cfg3 = MCP_8MHz_80kBPS_CFG3;
            break;

            case (CAN_100KBPS):                                             // 100Kbps
            cfg1 = MCP_8MHz_100kBPS_CFG1;
            cfg2 = MCP_8MHz_100kBPS_CFG2;
            cfg3 = MCP_8MHz_100kBPS_CFG3;
            break;

            case (CAN_125KBPS):                                             // 125Kbps
            cfg1 = MCP_8MHz_125kBPS_CFG1;
            cfg2 = MCP_8MHz_125kBPS_CFG2;
            cfg3 = MCP_8MHz_125kBPS_CFG3;
            break;

            case (CAN_200KBPS):                                             // 200Kbps
            cfg1 = MCP_8MHz_200kBPS_CFG1;
            cfg2 = MCP_8MHz_200kBPS_CFG2;
            cfg3 = MCP_8MHz_200kBPS_CFG3;
            break;

            case (CAN_250KBPS):                                             // 250Kbps
            cfg1 = MCP_8MHz_250kBPS_CFG1;
            cfg2 = MCP_8MHz_250kBPS_CFG2;
            cfg3 = MCP_8MHz_250kBPS_CFG3;
            break;

            case (CAN_500KBPS):                                             // 500Kbps
            cfg1 = MCP_8MHz_500kBPS_CFG1;
            cfg2 = MCP_8MHz_500kBPS_CFG2;
            cfg3 = MCP_8MHz_500kBPS_CFG3;
            break;

            case (CAN_1000KBPS):                                            //   1Mbps
            cfg1 = MCP_8MHz_1000kBPS_CFG1;
            cfg2 = MCP_8MHz_1000kBPS_CFG2;
            cfg3 = MCP_8MHz_1000kBPS_CFG3;
            break;

            default:
            set = 0;
            break;
        }
        break;

        case (MCP_16MHZ):
        switch (canSpeed)
        {
            case (CAN_5KBPS):                                               //   5Kbps
            cfg1 = MCP_16MHz_5kBPS_CFG1;
            cfg2 = MCP_16MHz_5kBPS_CFG2;
            cfg3 = MCP_16MHz_5kBPS_CFG3;
            break;

            case (CAN_10KBPS):                                              //  10Kbps
            cfg1 = MCP_16MHz_10kBPS_CFG1;
            cfg2 = MCP_16MHz_10kBPS_CFG2;
            cfg3 = MCP_16MHz_10kBPS_CFG3;
            break;

            case (CAN_20KBPS):                                              //  20Kbps
            cfg1 = MCP_16MHz_20kBPS_CFG1;
            cfg2 = MCP_16MHz_20kBPS_CFG2;
            cfg3 = MCP_16MHz_20kBPS_CFG3;
            break;

            case (CAN_33KBPS):                                              //  33.333Kbps
            cfg1 = MCP_16MHz_33k3BPS_CFG1;
            cfg2 = MCP_16MHz_33k3BPS_CFG2;
            cfg3 = MCP_16MHz_33k3BPS_CFG3;
            break;

            case (CAN_40KBPS):                                              //  40Kbps
            cfg1 = MCP_16MHz_40kBPS_CFG1;
            cfg2 = MCP_16MHz_40kBPS_CFG2;
            cfg3 = MCP_16MHz_40kBPS_CFG3;
            break;

            case (CAN_50KBPS):                                              //  50Kbps
            cfg1 = MCP_16MHz_50kBPS_CFG1;
            cfg2 = MCP_16MHz_50kBPS_CFG2;
            cfg3 = MCP_16MHz_50kBPS_CFG3;
            break;

            case (CAN_80KBPS):                                              //  80Kbps
            cfg1 = MCP_16MHz_80kBPS_CFG1;
            cfg2 = MCP_16MHz_80kBPS_CFG2;
            cfg3 = MCP_16MHz_80kBPS_CFG3;
            break;

            case (CAN_83K3BPS):                                             //  83.333Kbps
            cfg1 = MCP_16MHz_83k3BPS_CFG1;
            cfg2 = MCP_16MHz_83k3BPS_CFG2;
            cfg3 = MCP_16MHz_83k3BPS_CFG3;
            break; 

            case (CAN_100KBPS):                                             // 100Kbps
            cfg1 = MCP_16MHz_100kBPS_CFG1;
            cfg2 = MCP_16MHz_100kBPS_CFG2;
            cfg3 = MCP_16MHz_100kBPS_CFG3;
            break;

            case (CAN_125KBPS):                                             // 125Kbps
            cfg1 = MCP_16MHz_125kBPS_CFG1;
            cfg2 = MCP_16MHz_125kBPS_CFG2;
            cfg3 = MCP_16MHz_125kBPS_CFG3;
            break;

            case (CAN_200KBPS):                                             // 200Kbps
            cfg1 = MCP_16MHz_200kBPS_CFG1;
            cfg2 = MCP_16MHz_200kBPS_CFG2;
            cfg3 = MCP_16MHz_200kBPS_CFG3;
            break;

            case (CAN_250KBPS):                                             // 250Kbps
            cfg1 = MCP_16MHz_250kBPS_CFG1;
            cfg2 = MCP_16MHz_250kBPS_CFG2;
            cfg3 = MCP_16MHz_250kBPS_CFG3;
            break;

            case (CAN_500KBPS):                                             // 500Kbps
            cfg1 = MCP_16MHz_500kBPS_CFG1;
            cfg2 = MCP_16MHz_500kBPS_CFG2;
            cfg3 = MCP_16MHz_500kBPS_CFG3;
            break;

            case (CAN_1000KBPS):                                            //   1Mbps
            cfg1 = MCP_16MHz_1000kBPS_CFG1;
            cfg2 = MCP_16MHz_1000kBPS_CFG2;
            cfg3 = MCP_16MHz_1000kBPS_CFG3;
            break;

            default:
            set = 0;
            break;
        }
        break;

        case (MCP_20MHZ):
        switch (canSpeed)
        {
            case (CAN_33KBPS):                                              //  33.333Kbps
            cfg1 = MCP_20MHz_33k3BPS_CFG1;
            cfg2 = MCP_20MHz_33k3BPS_CFG2;
            cfg3 = MCP_20MHz_33k3BPS_CFG3;
	    break;

            case (CAN_40KBPS):                                              //  40Kbps
            cfg1 = MCP_20MHz_40kBPS_CFG1;
            cfg2 = MCP_20MHz_40kBPS_CFG2;
            cfg3 = MCP_20MHz_40kBPS_CFG3;
            break;

            case (CAN_50KBPS):                                              //  50Kbps
            cfg1 = MCP_20MHz_50kBPS_CFG1;
            cfg2 = MCP_20MHz_50kBPS_CFG2;
            cfg3 = MCP_20MHz_50kBPS_CFG3;
            break;

            case (CAN_80KBPS):                                              //  80Kbps
            cfg1 = MCP_20MHz_80kBPS_CFG1;
            cfg2 = MCP_20MHz_80kBPS_CFG2;
            cfg3 = MCP_20MHz_80kBPS_CFG3;
            break;

            case (CAN_83K3BPS):                                             //  83.333Kbps
            cfg1 = MCP_20MHz_83k3BPS_CFG1;
            cfg2 = MCP_20MHz_83k3BPS_CFG2;
            cfg3 = MCP_20MHz_83k3BPS_CFG3;
	    break;

            case (CAN_100KBPS):                                             // 100Kbps
            cfg1 = MCP_20MHz_100kBPS_CFG1;
            cfg2 = MCP_20MHz_100kBPS_CFG2;
            cfg3 = MCP_20MHz_100kBPS_CFG3;
            break;

            case (CAN_125KBPS):                                             // 125Kbps
            cfg1 = MCP_20MHz_125kBPS_CFG1;
            cfg2 = MCP_20MHz_125kBPS_CFG2;
            cfg3 = MCP_20MHz_125kBPS_CFG3;
            break;

            case (CAN_200KBPS):                                             // 200Kbps
            cfg1 = MCP_20MHz_200kBPS_CFG1;
            cfg2 = MCP_20MHz_200kBPS_CFG2;
            cfg3 = MCP_20MHz_200kBPS_CFG3;
            break;

            case (CAN_250KBPS):                                             // 250Kbps
            cfg1 = MCP_20MHz_250kBPS_CFG1;
            cfg2 = MCP_20MHz_250kBPS_CFG2;
            cfg3 = MCP_20MHz_250kBPS_CFG3;
            break;

            case (CAN_500KBPS):                                             // 500Kbps
            cfg1 = MCP_20MHz_500kBPS_CFG1;
            cfg2 = MCP_20MHz_500kBPS_CFG2;
            cfg3 = MCP_20MHz_500kBPS_CFG3;
            break;

            case (CAN_1000KBPS):                                            //   1Mbps
            cfg1 = MCP_20MHz_1000kBPS_CFG1;
            cfg2 = MCP_20MHz_1000kBPS_CFG2;
            cfg3 = MCP_20MHz_1000kBPS_CFG3;
            break;

            default:
            set = 0;
            break;
        }
        break;

        default:
        set = 0;
        break;
    }

    if (set) {
        private_setRegister(mcp2515, MCP_CNF1, cfg1);
        private_setRegister(mcp2515, MCP_CNF2, cfg2);
        private_setRegister(mcp2515, MCP_CNF3, cfg3);
        return ERROR_OK;
    }
    else {
        return ERROR_FAIL;
    }
}

ERROR setClkOut(MCP2515* mcp2515, const CAN_CLKOUT divisor)
{
    if (divisor == CLKOUT_DISABLE) {
	/* Turn off CLKEN */
	private_modifyRegister(mcp2515, MCP_CANCTRL, CANCTRL_CLKEN, 0x00);

	/* Turn on CLKOUT for SOF */
	private_modifyRegister(mcp2515, MCP_CNF3, CNF3_SOF, CNF3_SOF);
        return ERROR_OK;
    }

    /* Set the prescaler (CLKPRE) */
    private_modifyRegister(mcp2515, MCP_CANCTRL, CANCTRL_CLKPRE, divisor);

    /* Turn on CLKEN */
    private_modifyRegister(mcp2515, MCP_CANCTRL, CANCTRL_CLKEN, CANCTRL_CLKEN);

    /* Turn off CLKOUT for SOF */
    private_modifyRegister(mcp2515, MCP_CNF3, CNF3_SOF, 0x00);
    return ERROR_OK;
}

ERROR setFilterMask(MCP2515* mcp2515, const MASK mask, const bool ext, const uint32_t ulData)
{
    ERROR res = setConfigMode(mcp2515);
    if (res != ERROR_OK) {
        return res;
    }
    
    uint8_t tbufdata[4];
    private_prepareId(tbufdata, ext, ulData);

    REGISTER reg;
    switch (mask) {
        case MASK0: reg = MCP_RXM0SIDH; break;
        case MASK1: reg = MCP_RXM1SIDH; break;
        default:
            return ERROR_FAIL;
    }

    private_setRegistersManual(mcp2515, reg, tbufdata, 4);
    
    return ERROR_OK;
}

ERROR setFilter(MCP2515* mcp2515, const RXF num, const bool ext, const uint32_t ulData)
{
    ERROR res = setConfigMode(mcp2515);
    if (res != ERROR_OK) {
        return res;
    }

    REGISTER reg;

    switch (num) {
        case RXF0: reg = MCP_RXF0SIDH; break;
        case RXF1: reg = MCP_RXF1SIDH; break;
        case RXF2: reg = MCP_RXF2SIDH; break;
        case RXF3: reg = MCP_RXF3SIDH; break;
        case RXF4: reg = MCP_RXF4SIDH; break;
        case RXF5: reg = MCP_RXF5SIDH; break;
        default:
            return ERROR_FAIL;
    }

    uint8_t tbufdata[4];
    private_prepareId(tbufdata, ext, ulData);
    private_setRegistersManual(mcp2515, reg, tbufdata, 4);

    return ERROR_OK;
}

ERROR sendMessageManual(MCP2515* mcp2515, const TXBn txbn, const struct can_frame *frame)
{
    if (frame->can_dlc > CAN_MAX_DLEN) {
        return ERROR_FAILTX;
    }

    const struct TXBn_REGS *txbuf = &TXB[txbn];

    uint8_t data[13];

    bool ext = (frame->can_id & CAN_EFF_FLAG);
    bool rtr = (frame->can_id & CAN_RTR_FLAG);
    uint32_t id = (frame->can_id & (ext ? CAN_EFF_MASK : CAN_SFF_MASK));

    private_prepareId(data, ext, id);

    data[MCP_DLC] = rtr ? (frame->can_dlc | RTR_MASK) : frame->can_dlc;

    memcpy(&data[MCP_DATA], frame->data, frame->can_dlc);

    private_setRegistersManual(mcp2515, txbuf->SIDH, data, 5 + frame->can_dlc);

    private_modifyRegister(mcp2515, txbuf->CTRL, TXB_TXREQ, TXB_TXREQ);

    uint8_t ctrl = private_readRegister(mcp2515, txbuf->CTRL);
    if ((ctrl & (TXB_ABTF | TXB_MLOA | TXB_TXERR)) != 0) {
        return ERROR_FAILTX;
    }
    return ERROR_OK;
}

ERROR sendMessage(MCP2515* mcp2515, const struct can_frame *frame)
{
    if (frame->can_dlc > CAN_MAX_DLEN) {
        return ERROR_FAILTX;
    }

    TXBn txBuffers[N_TXBUFFERS] = {TXB0, TXB1, TXB2};

    for (int i = 0; i < N_TXBUFFERS; i++) {
        const struct TXBn_REGS *txbuf = &TXB[txBuffers[i]];
        uint8_t ctrlval = private_readRegister(mcp2515, txbuf->CTRL);
        if ( (ctrlval & TXB_TXREQ) == 0 ) {
            return sendMessageManual(mcp2515, txBuffers[i], frame);
        }
    }

    return ERROR_ALLTXBUSY;
}

ERROR readMessageManual(MCP2515* mcp2515, const RXBn rxbn, struct can_frame *frame)
{
    const struct RXBn_REGS *rxb = &RXB[rxbn];

    uint8_t tbufdata[5];

    private_readRegistersManual(mcp2515, rxb->SIDH, tbufdata, 5);

    uint32_t id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);

    if ( (tbufdata[MCP_SIDL] & TXB_EXIDE_MASK) ==  TXB_EXIDE_MASK ) {
        id = (id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        id = (id<<8) + tbufdata[MCP_EID8];
        id = (id<<8) + tbufdata[MCP_EID0];
        id |= CAN_EFF_FLAG;
    }

    uint8_t dlc = (tbufdata[MCP_DLC] & DLC_MASK);
    if (dlc > CAN_MAX_DLEN) {
        return ERROR_FAIL;
    }

    uint8_t ctrl = private_readRegister(mcp2515, rxb->CTRL);
    if (ctrl & RXBnCTRL_RTR) {
        id |= CAN_RTR_FLAG;
    }

    frame->can_id = id;
    frame->can_dlc = dlc;

    private_readRegistersManual(mcp2515, rxb->DATA, frame->data, dlc);

    private_modifyRegister(mcp2515, MCP_CANINTF, rxb->CANINTF_RXnIF, 0);

    return ERROR_OK;
}

ERROR readMessage(MCP2515* mcp2515, struct can_frame *frame)
{
    ERROR rc;
    uint8_t stat = getStatus(mcp2515);

    if ( stat & STAT_RX0IF ) {
        rc = readMessageManual(mcp2515, RXB0, frame);
    } else if ( stat & STAT_RX1IF ) {
        rc = readMessageManual(mcp2515, RXB1, frame);
    } else {
        rc = ERROR_MSGLOST;
    }

    return rc;
}

bool checkReceive(MCP2515* mcp2515)
{
    uint8_t res = getStatus(mcp2515);
    if ( res & STAT_RXIF_MASK ) {
        return true;
    } else {
        return false;
    }
}

bool checkError(MCP2515* mcp2515)
{
    uint8_t eflg = getErrorFlags(mcp2515);

    if ( eflg & EFLG_ERRORMASK) {
        return true;
    } else {
        return false;
    }
}

uint8_t getErrorFlags(MCP2515* mcp2515)
{
    return private_readRegister(mcp2515, MCP_EFLG);
}

void clearRXnOVRFlags(MCP2515* mcp2515)
{
	private_modifyRegister(mcp2515, MCP_EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0);
}

uint8_t getInterrupts(MCP2515* mcp2515)
{
    return private_readRegister(mcp2515, MCP_CANINTF);
}

void clearInterrupts(MCP2515* mcp2515)
{
    private_setRegister(mcp2515, MCP_CANINTF, 0);
}

uint8_t getInterruptMask(MCP2515* mcp2515)
{
    return private_readRegister(mcp2515, MCP_CANINTE);
}

void clearTXInterrupts(MCP2515* mcp2515)
{
    private_modifyRegister(mcp2515, MCP_CANINTF, (CANINTF_TX0IF | CANINTF_TX1IF | CANINTF_TX2IF), 0);
}

void clearRXnOVR(MCP2515* mcp2515)
{
	uint8_t eflg = getErrorFlags(mcp2515);
	if (eflg != 0) {
		clearRXnOVRFlags(mcp2515);
		clearInterrupts(mcp2515);
		//modifyRegister(MCP_CANINTF, CANINTF_ERRIF, 0);
	}
	
}

void clearMERR(MCP2515* mcp2515)
{
	//modifyRegister(MCP_EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0);
	//clearInterrupts();
	private_modifyRegister(mcp2515, MCP_CANINTF, CANINTF_MERRF, 0);
}

void clearERRIF(MCP2515* mcp2515)
{
    //modifyRegister(MCP_EFLG, EFLG_RX0OVR | EFLG_RX1OVR, 0);
    //clearInterrupts();
    private_modifyRegister(mcp2515, MCP_CANINTF, CANINTF_ERRIF, 0);
}

uint8_t errorCountRX(MCP2515* mcp2515)                             
{
    return private_readRegister(mcp2515, MCP_REC);
}

uint8_t errorCountTX(MCP2515* mcp2515)                             
{
    return private_readRegister(mcp2515, MCP_TEC);
}

