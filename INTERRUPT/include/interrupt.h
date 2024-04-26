//#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "MCP2515.h"
#include "canTransmit.h"
#define MCPInterruptPin 22

extern float MatherBoardTelemtry[47];
extern MCP2515 can0;
namespace INTERRUPT {
    void MCPInterruptSetup();
}