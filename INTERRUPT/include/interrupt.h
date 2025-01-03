#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "MCP2515.h"
#include "canTransmit.h"

#define MCPInterruptPin 22

extern uint16_t MatherBoardTelemtry[62];
extern MCP2515 can0;
extern uint32_t last_reset_time;
union uint32Time{
    uint32_t data;
    struct{
        uint16_t data1;
        uint16_t data2;
    };
};
namespace INTERRUPT {
    void MCPInterruptSetup();
}