#include "pico/stdlib.h"
#include "MCP2515.h"
namespace CAN{
    int CanPreparation(can_frame &canFrame, uint8_t needToByte, float data);
}