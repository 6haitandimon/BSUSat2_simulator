#include "pico/stdlib.h"
#include "MCP2515.h"
namespace CAN{
    int8_t CanPreparation(can_frame &canFrame, uint8_t needToByte, uint16_t data);
}