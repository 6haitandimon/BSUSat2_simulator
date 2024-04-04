//#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "stdio.h"
#include "MCP2515.h"
#define MCPInterruptPin 22

extern MCP2515 can0;
void MCPInterruptSetup();
